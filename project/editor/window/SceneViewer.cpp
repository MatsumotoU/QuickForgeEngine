#include "SceneViewer.h"
#include "inspector/ComponentInspectorRegistry.h"
#include "AssetDrop.h"
#include "command/ChangeTransformCommand.h"
#include "command/EditorCommandList.h"
#include "ecs/entity/EntityManager.h"
#include "components/TransformComponent.h"
#include "components/TransformHierarchy.h"
#include "framework/scene/SceneCoreFramework.h"
#include "framework/assets/AssetsCoreFramework.h"
#include "components/ObjectInfoComponent.h"
#include "ecs/component/JsonArchive.h"
#include "components/ModelRenderComponent.h"
#include "components/ParentComponent.h"
#include "command/IEditorCommand.h"
#include "command/CreateEntityCommand.h"
#include "command/AddSavedEntitiesCommand.h"
#include "string/MyString.h"
#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <utility>
#include <cmath>

namespace {
	using QFE::MATH::Vector3;
	using Snapshot = std::vector<std::pair<uint32_t, nlohmann::json>>;
	std::filesystem::path Utf8Path(const std::string& text) {
		return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(text.data()), text.size()));
	}

	// 追加した全エンティティを保存しておき、Redo でも UUID と参照関係を保つ。
	void RestorePainted(QFE::EntityManager& entities, const Snapshot& snapshot) {
		for (const auto& [id, data] : snapshot) {
			(void)data;
			entities.CancelEntityRemoval(id);
			if (!entities.IsActiveEntity(id)) entities.ForceCreateEntity(id);
		}
		for (const auto& [id, data] : snapshot) entities.DeserializeEntityComponents(id, data);
	}
	void RemovePainted(QFE::EntityManager& entities, const Snapshot& snapshot) {
		for (const auto& [id, data] : snapshot) { (void)data; if (entities.IsActiveEntity(id)) entities.RemoveEntity(id); }
	}
	class PaintCommand final : public QFE::EDITOR::IEditorCommand {
	public:
		PaintCommand(QFE::SCENE::SceneManager* scene, std::string path, Vector3 origin, bool entity,
			bool overrideGroup, std::string hierarchyGroup)
			: scene_(scene), path_(std::move(path)), origin_(origin), entity_(entity),
			overrideGroup_(overrideGroup), hierarchyGroup_(std::move(hierarchyGroup)) {}
		void Execute() override {
			auto& manager = QFE::FRAMEWORK::GetSceneEntities(*scene_);
			if (!snapshot_.empty()) { RestorePainted(manager, snapshot_); return; }
			const auto before = manager.GetActiveEntityIds();
			const std::unordered_set<uint32_t> old(before.begin(), before.end());
			if (entity_) {
				const auto fullPath = QFE::FRAMEWORK::ASSETS::GetProjectAssets() / Utf8Path(path_);
				QFE::EDITOR::AddSavedEntitiesCommand add(scene_, QFE::ConvertString(fullPath.wstring()), origin_);
				add.Execute();
			} else {
				const auto path = Utf8Path(path_);
				const auto stem = path.stem().generic_u8string();
				QFE::EDITOR::CreateEntityCommand add(std::string(reinterpret_cast<const char*>(stem.data()), stem.size()), origin_, &manager, path_);
				add.Execute();
			}
			for (uint32_t id : manager.GetActiveEntityIds()) {
				if (old.contains(id)) continue;
				if (overrideGroup_) {
					// 指定した場合は複数エンティティのアセットも全て同じ表示グループへ入れる。
					auto* info = manager.GetComponentPtr<QFE::SCENE::ObjectInfoComponent>(id);
					if (!info) {
						info = &manager.AddDefaultComponent<QFE::SCENE::ObjectInfoComponent>(id);
						const auto name = Utf8Path(path_).stem().generic_u8string();
						info->name.assign(reinterpret_cast<const char*>(name.data()), name.size());
					}
					info->hierarchyGroup = hierarchyGroup_;
				}
				snapshot_.emplace_back(id, manager.SerializeEntityComponents(id));
			}
		}
		void Undo() override { RemovePainted(QFE::FRAMEWORK::GetSceneEntities(*scene_), snapshot_); }
	private:
		QFE::SCENE::SceneManager* scene_;
		std::string path_;
		Vector3 origin_;
		bool entity_;
		bool overrideGroup_;
		std::string hierarchyGroup_;
		Snapshot snapshot_;
	};
	class ErasePaintCommand final : public QFE::EDITOR::IEditorCommand {
	public:
		ErasePaintCommand(QFE::SCENE::SceneManager* scene, Snapshot snapshot)
			: scene_(scene), snapshot_(std::move(snapshot)) {}
		void Execute() override { RemovePainted(QFE::FRAMEWORK::GetSceneEntities(*scene_), snapshot_); }
		void Undo() override { RestorePainted(QFE::FRAMEWORK::GetSceneEntities(*scene_), snapshot_); }
	private:
		QFE::SCENE::SceneManager* scene_;
		Snapshot snapshot_;
	};

	// 同次クリップ空間で線分を切り、カメラ背面や画面外への線の伸びを防ぐ。
	bool ClipGridLine(QFE::MATH::Vector4& a, QFE::MATH::Vector4& b) {
		const auto distance = [](const QFE::MATH::Vector4& v, int plane) {
			switch (plane) {
			case 0: return v.w + v.x;
			case 1: return v.w - v.x;
			case 2: return v.w + v.y;
			case 3: return v.w - v.y;
			case 4: return v.z;
			default: return v.w - v.z;
			}
		};
		for (int plane = 0; plane < 6; ++plane) {
			const float da = distance(a, plane), db = distance(b, plane);
			if (!std::isfinite(da) || !std::isfinite(db) || (da < 0.0f && db < 0.0f)) return false;
			if (da < 0.0f) a = a + (b - a) * (da / (da - db));
			else if (db < 0.0f) b = b + (a - b) * (db / (db - da));
		}
		return a.w > 1.0e-6f && b.w > 1.0e-6f;
	}
}

QFE::EDITOR::SceneViewer::SceneViewer(ImTextureID sceneTextureId, SCENE::SceneManager* sceneManager, ASSET::ModelLoader& modelLoader, unsigned width, unsigned height,
	const ComponentInspectorRegistry* componentInspectorRegistry) :
	sceneManager_(sceneManager),
	componentInspectorRegistry_(componentInspectorRegistry),
	sceneTextureId_(sceneTextureId),
	width_(width), height_(height),
	isActive_(true),
	isFocus_(false),
	isImageHovered_(false),
	isOrthographic_(false),
	wasUsingGizmo_(false),
	editingEntityId_(0),
	gizmoOperation_(ImGuizmo::TRANSLATE),
	gizmoMode_(ImGuizmo::LOCAL),
	viewMatrix_(QFE::MATH::Matrix4x4::MakeIdentity4x4()),
	projectionMatrix_(QFE::MATH::Matrix4x4::MakeIdentity4x4()), modelLoader_(&modelLoader) {
}

void QFE::EDITOR::SceneViewer::Initialize() {
	isActive_ = true;
}

void QFE::EDITOR::SceneViewer::Draw(std::set<uint32_t>& selectedEntities, EditorCommandList& commandList) {
	ImGui::Begin(GetWindowName().c_str(), &isActive_, placementMode_ != 0 ? ImGuiWindowFlags_NoScrollWithMouse : 0);
	const float targetAspectRatio = static_cast<float>(width_) / static_cast<float>(height_);
	isFocus_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	const char* modes[] = { "View", "Entity Paint", "Model Paint" };
	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::Combo("Mode", &placementMode_, modes, IM_ARRAYSIZE(modes))) {
		placementIndex_ = 0;
		hasLastPaintPosition_ = false;
		RefreshPlacementAssets();
		LoadPlacementPreview();
		if (!placementAssets_.empty()) changedPaintSelection_ = placementAssets_[placementIndex_];
	}
	if (placementMode_ != 0) {
		ImGui::SameLine();
		if (ImGui::Button("Refresh Assets")) {
			RefreshPlacementAssets();
			LoadPlacementPreview();
			if (!placementAssets_.empty()) changedPaintSelection_ = placementAssets_[placementIndex_];
		}
		std::set<std::string> groups;
		auto& entities = QFE::FRAMEWORK::GetSceneEntities(*sceneManager_);
		entities.Each<QFE::SCENE::ObjectInfoComponent>([&](uint32_t, QFE::SCENE::ObjectInfoComponent& info) {
			if (!info.hierarchyGroup.empty()) groups.insert(info.hierarchyGroup);
		});
		// グループ内の最後のエンティティを消しても、次の配置先として保持する。
		if (!useAssetGroups_ && !placementGroup_.empty()) groups.insert(placementGroup_);
		const char* currentGroup = useAssetGroups_ ? "Asset's groups" :
			(placementGroup_.empty() ? "Ungrouped" : placementGroup_.c_str());
		ImGui::SetNextItemWidth(220.0f);
		if (ImGui::BeginCombo("Placement group", currentGroup)) {
			if (ImGui::Selectable("Asset's groups", useAssetGroups_)) useAssetGroups_ = true;
			if (ImGui::Selectable("Ungrouped", !useAssetGroups_ && placementGroup_.empty())) {
				useAssetGroups_ = false;
				placementGroup_.clear();
			}
			for (const auto& group : groups) {
				ImGui::PushID(group.c_str());
				if (ImGui::Selectable(group.c_str(), !useAssetGroups_ && placementGroup_ == group)) {
					useAssetGroups_ = false;
					placementGroup_ = group;
				}
				ImGui::PopID();
			}
			ImGui::EndCombo();
		}
		if (!useAssetGroups_) {
			ImGui::SetNextItemWidth(120.0f);
			if (ImGui::DragFloat("Erase radius", &eraseRadius_, 0.05f, 0.0f, 1000.0f, "%.2f")) {
				eraseRadius_ = std::isfinite(eraseRadius_) ? std::clamp(eraseRadius_, 0.0f, 1000.0f) : 0.5f;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("World distance from the grid point to the entity origin. Deletes the nearest matching entity.");
		}
		// ファイル名とパスを明示し、同名のアセットでも区別できるようにする。
		if (placementAssets_.empty()) {
			ImGui::TextDisabled("Paint target: No asset selected");
		} else {
			ImGui::TextColored(ImVec4(0.45f, 0.85f, 1.0f, 1.0f), "Paint target: %s", placementAssets_[placementIndex_].c_str());
		}
		ImGui::TextDisabled("Wheel: choose  Right drag: place  Left drag: erase  Middle drag: camera");
		if (!placementStatus_.empty()) ImGui::TextWrapped("%s", placementStatus_.c_str());
	}
	DrawGridControls();
	ImGui::Checkbox("Game Debug", &showGameOverlays_);

	ImVec2 availSize = ImGui::GetContentRegionAvail();
	if (availSize.x <= 0.0f || availSize.y <= 0.0f) {
		isImageHovered_ = false;
		ImGui::End();
		return;
	}

	ImVec2 renderSize;
	if (availSize.x / availSize.y > targetAspectRatio) {
		renderSize.y = availSize.y;
		renderSize.x = availSize.y * targetAspectRatio;
	} else {
		renderSize.x = availSize.x;
		renderSize.y = availSize.x / targetAspectRatio;
	}

	const ImVec2 contentStart = ImGui::GetCursorScreenPos();
	const ImVec2 imagePosition(
		contentStart.x + (availSize.x - renderSize.x) * 0.5f,
		contentStart.y + (availSize.y - renderSize.y) * 0.5f);
	ImGui::SetCursorScreenPos(imagePosition);
	ImGui::Image(sceneTextureId_, renderSize);
	isImageHovered_ = ImGui::IsItemHovered();
	AcceptAssetDrop(sceneManager_, commandList, imagePosition, renderSize, width_, height_,
		MATH::Matrix4x4::MakeIdentity4x4(), dropViewProjection_, &grid_);
	if (debugLines_) FRAMEWORK::DrawDebugLines(*debugLines_, dropViewProjection_,
		{ imagePosition.x, imagePosition.y }, { renderSize.x, renderSize.y }, false);
	if (grid_.enabled || placementMode_ != 0) DrawPlacementGrid(imagePosition, renderSize);
	if (showGameOverlays_ && componentInspectorRegistry_ && sceneManager_) {
		componentInspectorRegistry_->DrawSceneOverlays(QFE::FRAMEWORK::GetSceneEntities(*sceneManager_),
			dropViewProjection_, imagePosition, renderSize);
	}
	if (placementMode_ != 0) DrawPlacementTools(imagePosition, renderSize, commandList);

	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::SetRect(imagePosition.x, imagePosition.y, renderSize.x, renderSize.y);
	ImGuizmo::SetOrthographic(isOrthographic_);

	if (placementMode_ == 0) {
		DrawGizmoToolbar(isImageHovered_);
		DrawGizmo(selectedEntities, commandList);
	}

	ImGui::End();
}

void QFE::EDITOR::SceneViewer::DrawGridControls() {
	ImGui::Checkbox("Grid Snap", &grid_.enabled);
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("Snap asset placement and gizmo translation to the world grid.");
	if (!grid_.enabled && placementMode_ == 0) return;
	ImGui::SameLine();
	ImGui::SetNextItemWidth(78.0f);
	const char* names[] = { "XZ", "XY", "YZ" };
	int plane = static_cast<int>(grid_.plane);
	if (ImGui::Combo("Plane", &plane, names, IM_ARRAYSIZE(names))) grid_.plane = static_cast<GridPlane>(plane);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90.0f);
	if (ImGui::DragFloat("Spacing", &grid_.spacing, 0.05f, 0.01f, 1000.0f, "%.2f")) {
		grid_.spacing = std::isfinite(grid_.spacing) ? std::clamp(grid_.spacing, 0.01f, 1000.0f) : 1.0f;
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(75.0f);
	const char* layerLabel = grid_.plane == GridPlane::XZ ? "Y Layer" : grid_.plane == GridPlane::XY ? "Z Layer" : "X Layer";
	if (ImGui::DragInt(layerLabel, &grid_.layer, 1.0f, -1000, 1000)) {
		grid_.layer = std::clamp(grid_.layer, -1000, 1000);
	}
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("Plane offset = layer x spacing, along the remaining world axis.");
}

void QFE::EDITOR::SceneViewer::DrawPlacementGrid(const ImVec2& imagePosition, const ImVec2& imageSize) {
	MATH::Vector3 center{};
	// 中央に面が見えなければ原点付近を表示する。配置可否はドロップ位置で別に判定する。
	IntersectGrid(grid_, dropViewProjection_, 0.5f, 0.5f, center);
	const float height = static_cast<float>(grid_.layer) * grid_.spacing;
	const auto point = [&](float u, float v) -> MATH::Vector3 {
		if (grid_.plane == GridPlane::XZ) return { u, height, v };
		if (grid_.plane == GridPlane::XY) return { u, v, height };
		return { height, u, v };
	};
	const float centerU = grid_.plane == GridPlane::YZ ? center.y : center.x;
	const float centerV = grid_.plane == GridPlane::XY ? center.y : center.z;
	const float extent = 20.0f * grid_.spacing;
	auto* draw = ImGui::GetWindowDrawList();
	draw->PushClipRect(imagePosition, ImVec2(imagePosition.x + imageSize.x, imagePosition.y + imageSize.y), true);
	const auto line = [&](const MATH::Vector3& start, const MATH::Vector3& end, bool major) {
		auto a = MATH::Vector4::EulerTransform({ start.x, start.y, start.z, 1.0f }, dropViewProjection_);
		auto b = MATH::Vector4::EulerTransform({ end.x, end.y, end.z, 1.0f }, dropViewProjection_);
		if (!ClipGridLine(a, b)) return;
		const auto screen = [&](const MATH::Vector4& p) {
			return ImVec2(imagePosition.x + (p.x / p.w + 1.0f) * 0.5f * imageSize.x,
				imagePosition.y + (1.0f - p.y / p.w) * 0.5f * imageSize.y);
		};
		draw->AddLine(screen(a), screen(b), major ? IM_COL32(160, 190, 220, 190) : IM_COL32(150, 170, 190, 105), major ? 1.5f : 1.0f);
	};
	for (int index = -20; index <= 20; ++index) {
		const float u = centerU + static_cast<float>(index) * grid_.spacing;
		const float v = centerV + static_cast<float>(index) * grid_.spacing;
		line(point(u, centerV - extent), point(u, centerV + extent), index % 10 == 0);
		line(point(centerU - extent, v), point(centerU + extent, v), index % 10 == 0);
	}
	const auto* payload = ImGui::GetDragDropPayload();
	const ImVec2 mouse = ImGui::GetMousePos();
	if (payload && payload->IsDataType(AssetPayloadType) &&
		mouse.x >= imagePosition.x && mouse.y >= imagePosition.y &&
		mouse.x < imagePosition.x + imageSize.x && mouse.y < imagePosition.y + imageSize.y) {
		MATH::Vector3 placement{};
		if (IntersectGrid(grid_, dropViewProjection_, (mouse.x - imagePosition.x) / imageSize.x,
			(mouse.y - imagePosition.y) / imageSize.y, placement)) {
			const auto projected = MATH::Vector4::EulerTransform({ placement.x, placement.y, placement.z, 1.0f }, dropViewProjection_);
			if (projected.w > 1.0e-6f) {
				const ImVec2 marker(imagePosition.x + (projected.x / projected.w + 1.0f) * 0.5f * imageSize.x,
					imagePosition.y + (1.0f - projected.y / projected.w) * 0.5f * imageSize.y);
				draw->AddCircleFilled(marker, 5.0f, IM_COL32(255, 210, 65, 240));
			}
		} else {
			ImGui::SetTooltip("The selected plane is parallel to the view or outside the camera range.");
		}
	}
	draw->PopClipRect();
}

void QFE::EDITOR::SceneViewer::RefreshPlacementAssets() {
	placementAssets_.clear();
	if (placementMode_ == 0) return;
	const auto root = QFE::FRAMEWORK::ASSETS::GetProjectAssets();
	std::error_code error;
	std::filesystem::recursive_directory_iterator it(root, std::filesystem::directory_options::skip_permission_denied, error), end;
	for (; !error && it != end; it.increment(error)) {
		if (!it->is_regular_file(error) || error || !QFE::FRAMEWORK::ASSETS::IsPathWithin(it->path(), root)) { error.clear(); continue; }
		const auto relative = it->path().lexically_relative(root);
		std::string ext = relative.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		const bool entity = (ext == ".entity" || ext == ".json") && relative.begin() != relative.end() && *relative.begin() == "entities";
		const bool model = ext == ".glb" || ext == ".obj";
		if ((placementMode_ == 1 && entity) || (placementMode_ == 2 && model)) {
			const auto utf8 = relative.generic_u8string();
			placementAssets_.emplace_back(reinterpret_cast<const char*>(utf8.data()), utf8.size());
		}
	}
	if (placementMode_ == 2) {
		const auto primitives = QFE::FRAMEWORK::ASSETS::GetPrimitiveMeshNames();
		placementAssets_.insert(placementAssets_.end(), primitives.begin(), primitives.end());
	}
	std::sort(placementAssets_.begin(), placementAssets_.end());
	placementAssets_.erase(std::unique(placementAssets_.begin(), placementAssets_.end()), placementAssets_.end());
	placementIndex_ = placementAssets_.empty() ? 0 : (std::min)(placementIndex_, static_cast<int>(placementAssets_.size()) - 1);
}

void QFE::EDITOR::SceneViewer::SelectPaintAsset(const std::string& path) {
	if (placementMode_ == 0 || path.empty()) return;
	const auto relative = Utf8Path(path);
	if (relative.is_absolute()) return;
	const auto root = QFE::FRAMEWORK::ASSETS::GetProjectAssets();
	std::error_code error;
	if (!QFE::FRAMEWORK::ASSETS::IsPathWithin(root / relative, root) ||
		!std::filesystem::is_regular_file(root / relative, error) || error) return;
	std::string extension = relative.extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	const bool entity = (extension == ".entity" || extension == ".json") &&
		relative.begin() != relative.end() && *relative.begin() == "entities";
	const bool model = extension == ".obj" || extension == ".glb";
	if (!entity && !model) return;
	const int previousMode = placementMode_;
	placementMode_ = entity ? 1 : 2;
	// 新しく追加されたアセットも、ブラウザで選ぶだけで配置候補に含める。
	RefreshPlacementAssets();
	const auto selected = std::find(placementAssets_.begin(), placementAssets_.end(), path);
	if (selected == placementAssets_.end()) {
		placementMode_ = previousMode;
		RefreshPlacementAssets();
		return;
	}
	placementIndex_ = static_cast<int>(std::distance(placementAssets_.begin(), selected));
	hasLastPaintPosition_ = false;
	LoadPlacementPreview();
}

std::string QFE::EDITOR::SceneViewer::ConsumePaintSelection() {
	return std::exchange(changedPaintSelection_, {});
}

void QFE::EDITOR::SceneViewer::LoadPlacementPreview() {
	previewTriangles_.clear();
	placementStatus_.clear();
	if (placementMode_ == 0 || placementAssets_.empty()) return;
	const auto addModel = [&](const std::string& modelName, const MATH::Matrix4x4& matrix) {
		std::vector<MATH::Vector3> vertices;
		std::string error;
		if (!QFE::FRAMEWORK::ASSETS::GetModelPreviewTriangles(*modelLoader_, modelName, vertices, error)) {
			placementStatus_ = "Preview unavailable: " + modelName + " " + error;
			return;
		}
		for (const auto& vertex : vertices) previewTriangles_.push_back(MATH::Vector3::EulerTransform(vertex, matrix));
	};
	if (placementMode_ == 2) {
		addModel(placementAssets_[placementIndex_], MATH::Matrix4x4::MakeIdentity4x4());
		return;
	}
	const auto path = QFE::FRAMEWORK::ASSETS::GetProjectAssets() / Utf8Path(placementAssets_[placementIndex_]);
	std::ifstream file(path);
	const auto json = nlohmann::json::parse(file, nullptr, false);
	if (!json.is_object()) {
		placementStatus_ = "Could not read entity asset.";
		return;
	}
	struct PreviewItem { SCENE::TransformComponent transform; std::string uuid; std::string parent; std::string model; };
	std::vector<PreviewItem> items;
	const auto entries = json.contains("entities") && json["entities"].is_array()
		? json["entities"] : nlohmann::json::array({ json });
	for (const auto& entry : entries) {
		if (!entry.is_object()) continue;
		auto components = entry.contains("components") ? entry["components"] : entry;
		if (!components.is_object()) continue;
		PreviewItem item{};
		try {
			if (components.contains("TransformComponent")) { auto value = components["TransformComponent"]; JsonArchive archive(value, true); item.transform.Reflect(archive); }
			if (components.contains("ModelRenderComponent")) { SCENE::ModelRenderComponent model; auto value = components["ModelRenderComponent"]; JsonArchive archive(value, true); model.Reflect(archive); item.model = model.modelName; }
			if (components.contains("ObjectInfoComponent")) item.uuid = components["ObjectInfoComponent"].value("uuid", std::string{});
			if (components.contains("ParentComponent")) {
				const auto& value = components["ParentComponent"]["parent"];
				if (value.is_string()) item.parent = value.get<std::string>();
				else if (value.is_object()) item.parent = value.value("uuid", std::string{});
			}
		} catch (const nlohmann::json::exception&) { placementStatus_ = "Invalid entity preview data."; continue; }
		items.push_back(std::move(item));
	}
	std::unordered_map<std::string, size_t> parents;
	for (size_t i = 0; i < items.size(); ++i) if (!items[i].uuid.empty()) parents[items[i].uuid] = i;
	for (size_t i = 0; i < items.size(); ++i) {
		if (items[i].model.empty()) continue;
		auto matrix = MATH::Matrix4x4::MakeAffineMatrix(items[i].transform.transform);
		std::unordered_set<size_t> visited{ i };
		std::string parent = items[i].parent;
		while (!parent.empty()) {
			const auto found = parents.find(parent);
			if (found == parents.end() || !visited.insert(found->second).second) break;
			matrix = MATH::Matrix4x4::Multiply(matrix, MATH::Matrix4x4::MakeAffineMatrix(items[found->second].transform.transform));
			parent = items[found->second].parent;
		}
		addModel(items[i].model, matrix);
	}
	if (previewTriangles_.empty() && placementStatus_.empty()) placementStatus_ = "No model to preview in this entity.";
}

void QFE::EDITOR::SceneViewer::DrawPlacementTools(const ImVec2& imagePosition, const ImVec2& imageSize, EditorCommandList& commandList) {
	const ImGuiIO& io = ImGui::GetIO();
	if (!isImageHovered_ || ImGui::IsMouseDown(ImGuiMouseButton_Middle)) { hasLastPaintPosition_ = false; return; }
	if (!placementAssets_.empty() && io.MouseWheel != 0.0f) {
		const int count = static_cast<int>(placementAssets_.size());
		placementIndex_ = (placementIndex_ + (io.MouseWheel > 0 ? -1 : 1) + count) % count;
		LoadPlacementPreview();
		changedPaintSelection_ = placementAssets_[placementIndex_];
	}
	const ImVec2 mouse = ImGui::GetMousePos();
	MATH::Vector3 position{};
	if (!IntersectGrid(grid_, dropViewProjection_, (mouse.x - imagePosition.x) / imageSize.x,
		(mouse.y - imagePosition.y) / imageSize.y, position)) return;
	// 実体ではない三角形をシーンビューに重ねるため、保存・衝突・Undo には影響しない。
	auto* draw = ImGui::GetWindowDrawList();
	draw->PushClipRect(imagePosition, ImVec2(imagePosition.x + imageSize.x, imagePosition.y + imageSize.y), true);
	// 高ポリゴンモデルでも ImGui の頂点数と描画負荷が急増しないよう間引く。
	const size_t triangleCount = previewTriangles_.size() / 3;
	const size_t stride = (std::max)(size_t{ 1 }, (triangleCount + 7999) / 8000);
	for (size_t i = 0; i + 2 < previewTriangles_.size(); i += 3 * stride) {
		ImVec2 points[3];
		bool visible = true;
		for (int j = 0; j < 3; ++j) {
			const auto p = previewTriangles_[i + j] + position;
			const auto clip = MATH::Vector4::EulerTransform({ p.x, p.y, p.z, 1.0f }, dropViewProjection_);
			if (clip.w <= 1.0e-5f || clip.z < 0.0f || clip.z > clip.w) { visible = false; break; }
			points[j] = { imagePosition.x + (clip.x / clip.w + 1.0f) * 0.5f * imageSize.x,
				imagePosition.y + (1.0f - clip.y / clip.w) * 0.5f * imageSize.y };
		}
		if (visible) draw->AddTriangleFilled(points[0], points[1], points[2], IM_COL32(80, 205, 255, 45));
	}
	draw->PopClipRect();
	const int button = ImGui::IsMouseDown(ImGuiMouseButton_Right) ? ImGuiMouseButton_Right :
		(ImGui::IsMouseDown(ImGuiMouseButton_Left) ? ImGuiMouseButton_Left : -1);
	if (button < 0) { hasLastPaintPosition_ = false; return; }
	if (hasLastPaintPosition_ && lastPaintButton_ == button && (position - lastPaintPosition_).LengthSq() < 1.0e-8f) return;
	hasLastPaintPosition_ = true; lastPaintPosition_ = position; lastPaintButton_ = button;
	if (button == ImGuiMouseButton_Right) {
		if (placementAssets_.empty()) return;
		commandList.AddCommand(std::make_unique<PaintCommand>(sceneManager_, placementAssets_[placementIndex_], position,
			placementMode_ == 1, !useAssetGroups_, placementGroup_));
		return;
	}
	auto& entities = QFE::FRAMEWORK::GetSceneEntities(*sceneManager_);
	if (!useAssetGroups_) {
		uint32_t nearestId = UINT32_MAX;
		float nearestDistance = eraseRadius_ * eraseRadius_;
		entities.Each<QFE::SCENE::ObjectInfoComponent, QFE::SCENE::TransformComponent>(
			[&](uint32_t id, QFE::SCENE::ObjectInfoComponent& info, QFE::SCENE::TransformComponent&) {
				if (info.hierarchyGroup != placementGroup_) return;
				// 親付きのエンティティも画面上と同じワールド位置で距離を測る。
				const auto world = QFE::SCENE::GetWorldMatrix(entities, id);
				const MATH::Vector3 origin{ world.m[3][0], world.m[3][1], world.m[3][2] };
				const float distance = (origin - position).LengthSq();
				if (distance <= nearestDistance && (distance < nearestDistance || id < nearestId)) {
					nearestId = id;
					nearestDistance = distance;
				}
			});
		if (nearestId == UINT32_MAX) return;
		Snapshot snapshot;
		std::unordered_set<uint32_t> groupIds;
		groupIds.insert(nearestId);
		snapshot.emplace_back(nearestId, entities.SerializeEntityComponents(nearestId));
		// 子孫は RemoveEntity により連鎖削除されるため、対象外のものを巻き込まない。
		for (uint32_t id : groupIds) for (uint32_t child : entities.GetDescendantEntityIds(id)) {
			if (!groupIds.contains(child)) {
				placementStatus_ = "Cannot erase: this entity has other child entities.";
				return;
			}
		}
		placementStatus_.clear();
		commandList.AddCommand(std::make_unique<ErasePaintCommand>(sceneManager_, std::move(snapshot)));
		return;
	}
	placementStatus_ = "Select a placement group or Ungrouped to erase.";
}

std::string QFE::EDITOR::SceneViewer::GetWindowName() {
	return "SceneViewer";
}

bool QFE::EDITOR::SceneViewer::GetIsActive() {
	return isActive_;
}

bool QFE::EDITOR::SceneViewer::SetIsActive(bool isActive) {
	isActive_ = isActive;
	return isActive_;
}

bool QFE::EDITOR::SceneViewer::GetIsFocus() {
	return isFocus_;
}

void QFE::EDITOR::SceneViewer::SetCameraMatrices(
	const QFE::MATH::Matrix4x4& viewMatrix,
	const QFE::MATH::Matrix4x4& projectionMatrix,
	bool isOrthographic) {
	viewMatrix_ = viewMatrix;
	projectionMatrix_ = projectionMatrix;
	isOrthographic_ = isOrthographic;
}

bool QFE::EDITOR::SceneViewer::IsGizmoCapturingMouse() const {
	return isImageHovered_ && (ImGuizmo::IsUsing() || ImGuizmo::IsOver());
}

void QFE::EDITOR::SceneViewer::SetAssetDropViewProjection(const MATH::Matrix4x4& viewProjection) {
	dropViewProjection_ = viewProjection;
}

void QFE::EDITOR::SceneViewer::DrawGizmoToolbar(bool imageHovered) {
	const ImGuiIO& io = ImGui::GetIO();
	if (imageHovered && !io.WantTextInput) {
		if (ImGui::IsKeyPressed(ImGuiKey_W)) {
			gizmoOperation_ = ImGuizmo::TRANSLATE;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_E)) {
			gizmoOperation_ = ImGuizmo::ROTATE;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_R)) {
			gizmoOperation_ = ImGuizmo::SCALE;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_X) && gizmoOperation_ != ImGuizmo::SCALE) {
			gizmoMode_ = gizmoMode_ == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
		}
	}

	if (imageHovered && ImGui::IsKeyPressed(ImGuiKey_LeftAlt)) {
		ImGui::OpenPopup("SceneGizmoMenu");
	}
	if (ImGui::BeginPopup("SceneGizmoMenu")) {
		if (ImGui::MenuItem("Translate (W)", nullptr, gizmoOperation_ == ImGuizmo::TRANSLATE)) {
			gizmoOperation_ = ImGuizmo::TRANSLATE;
		}
		if (ImGui::MenuItem("Rotate (E)", nullptr, gizmoOperation_ == ImGuizmo::ROTATE)) {
			gizmoOperation_ = ImGuizmo::ROTATE;
		}
		if (ImGui::MenuItem("Scale (R)", nullptr, gizmoOperation_ == ImGuizmo::SCALE)) {
			gizmoOperation_ = ImGuizmo::SCALE;
		}
		ImGui::Separator();
		const bool localMode = gizmoMode_ == ImGuizmo::LOCAL;
		if (ImGui::MenuItem("Local (X)", nullptr, localMode, gizmoOperation_ != ImGuizmo::SCALE)) {
			gizmoMode_ = ImGuizmo::LOCAL;
		}
		if (ImGui::MenuItem("World (X)", nullptr, !localMode, gizmoOperation_ != ImGuizmo::SCALE)) {
			gizmoMode_ = ImGuizmo::WORLD;
		}
		ImGui::EndPopup();
	}
}

void QFE::EDITOR::SceneViewer::DrawGizmo(
	std::set<uint32_t>& selectedEntities,
	EditorCommandList& commandList) {
	if (selectedEntities.size() != 1 || sceneManager_ == nullptr) {
		wasUsingGizmo_ = false;
		return;
	}

	const uint32_t entityId = *selectedEntities.begin();
	EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(*sceneManager_);
	if (!entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
		wasUsingGizmo_ = false;
		return;
	}

	auto& transformComponent =
		entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId);
	QFE::MATH::Matrix4x4 transformMatrix =
		QFE::SCENE::GetWorldMatrix(entityManager, entityId);

	ImGuizmo::SetID(static_cast<int>(entityId));
	const bool changed = ImGuizmo::Manipulate(
		&viewMatrix_.m[0][0],
		&projectionMatrix_.m[0][0],
		gizmoOperation_,
		gizmoOperation_ == ImGuizmo::SCALE ? ImGuizmo::LOCAL : gizmoMode_,
		&transformMatrix.m[0][0]);
	const bool isUsingGizmo = ImGuizmo::IsUsing();

	if (isUsingGizmo && !wasUsingGizmo_) {
		editingEntityId_ = entityId;
		transformBeforeEdit_ = transformComponent.transform;
	}

	if (changed && isUsingGizmo) {
		if (grid_.enabled && gizmoOperation_ == ImGuizmo::TRANSLATE) {
			// 親の変換を戻す前にワールド位置を丸め、親子の有無で格子をずらさない。
			const auto snapped = SnapGridPosition({ transformMatrix.m[3][0], transformMatrix.m[3][1], transformMatrix.m[3][2] }, grid_.spacing);
			transformMatrix.m[3][0] = snapped.x;
			transformMatrix.m[3][1] = snapped.y;
			transformMatrix.m[3][2] = snapped.z;
		}
		const QFE::MATH::Matrix4x4 parentWorldMatrix =
			QFE::SCENE::GetParentWorldMatrix(entityManager, entityId);
		const QFE::MATH::Matrix4x4 localMatrix =
			QFE::MATH::Matrix4x4::Multiply(transformMatrix, parentWorldMatrix.Inverse());
		transformComponent.transform.FromMatrix(localMatrix);
	}

	if (!isUsingGizmo && wasUsingGizmo_ && editingEntityId_ == entityId) {
		const QFE::MATH::EulerTransform transformAfterEdit = transformComponent.transform;
		if (transformAfterEdit != transformBeforeEdit_) {
			commandList.AddCommand(std::make_unique<ChangeTransformCommand>(
				entityId,
				&entityManager,
				transformBeforeEdit_,
				transformAfterEdit));
		}
	}

	wasUsingGizmo_ = isUsingGizmo;
}
