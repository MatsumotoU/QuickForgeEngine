#include "AssetDrop.h"
#include "GridPlacement.h"
#include "framework/assets/AssetsCoreFramework.h"
#include "command/AddSavedEntitiesCommand.h"
#include "command/CreateEntityCommand.h"
#include "command/EditorCommandList.h"
#include "ecs/entity/EntityManager.h"
#include "framework/scene/SceneCoreFramework.h"
#include "string/MyString.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <memory>

namespace {
enum class AssetKind { Unsupported, Entity, Scene, Model, Sprite };

std::filesystem::path PathFromUtf8(const std::string& text) {
	return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(text.data()), text.size()));
}

std::string ToUtf8(const std::filesystem::path& path) {
	const auto text = path.generic_u8string();
	return std::string(reinterpret_cast<const char*>(text.data()), text.size());
}

AssetKind GetAssetKind(const std::filesystem::path& path) {
	std::string extension = ToUtf8(path.extension());
	std::transform(extension.begin(), extension.end(), extension.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	if ((extension == ".entity" || extension == ".json") && path.begin() != path.end() && *path.begin() == "entities") return AssetKind::Entity;
	if ((extension == ".scene" || extension == ".json") && path.begin() != path.end() && *path.begin() == "scenes") return AssetKind::Scene;
	if (extension == ".obj" || extension == ".glb") return AssetKind::Model;
	if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
		extension == ".bmp" || extension == ".tif" || extension == ".tiff") return AssetKind::Sprite;
	return AssetKind::Unsupported;
}

}

bool QFE::EDITOR::IsDraggableAsset(const std::string& relativePath) {
	return GetAssetKind(PathFromUtf8(relativePath)) != AssetKind::Unsupported;
}

void QFE::EDITOR::AcceptAssetDrop(SCENE::SceneManager* sceneManager, EditorCommandList& commandList,
	const ImVec2& imagePosition, const ImVec2& imageSize, unsigned width, unsigned height,
	const MATH::Matrix4x4& view, const MATH::Matrix4x4& projection, const GridPlacement* grid) {
	if (!sceneManager || imageSize.x <= 0 || imageSize.y <= 0 || !ImGui::BeginDragDropTarget()) return;
	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(AssetPayloadType)) {
		if (payload->IsDelivery() && payload->DataSize > 1) {
			const auto* bytes = static_cast<const char*>(payload->Data);
			const std::string payloadData(bytes, bytes + payload->DataSize - 1);
			const std::string& relative = payloadData;
			const auto path = PathFromUtf8(relative);
			const auto assetPath = QFE::FRAMEWORK::ASSETS::GetProjectAssets() / path;
			std::error_code error;
			if (bytes[payload->DataSize - 1] == '\0' && !path.is_absolute() &&
				QFE::FRAMEWORK::ASSETS::IsPathWithin(assetPath, QFE::FRAMEWORK::ASSETS::GetProjectAssets()) &&
				std::filesystem::is_regular_file(assetPath, error) && !error) {
				const AssetKind kind = GetAssetKind(path);
				auto& manager = QFE::FRAMEWORK::GetSceneEntities(*sceneManager);
				const ImVec2 mouse = ImGui::GetMousePos();
				const float x = (mouse.x - imagePosition.x) / imageSize.x;
				const float y = (mouse.y - imagePosition.y) / imageSize.y;
				MATH::Vector3 gridPosition{};
				const bool gridEnabled = grid && grid->enabled;
				if (gridEnabled && !IntersectGrid(*grid, MATH::Matrix4x4::Multiply(view, projection), x, y, gridPosition)) {
					// 面が見えない方向では推測した位置に配置しない。
					QFE_LOG("Cannot place asset: the selected grid plane is not under the cursor.", QFE::LogLevel::Warning);
					ImGui::EndDragDropTarget();
					return;
				}
				if (kind == AssetKind::Entity) {
					commandList.AddCommand(std::make_unique<AddSavedEntitiesCommand>(sceneManager, QFE::ConvertString(assetPath.wstring()), gridPosition));
				} else if (kind == AssetKind::Scene) {
					// モデルと同じレイ上の位置を、追加シーンの原点とする。
					const auto inverse = MATH::Matrix4x4::Multiply(view, projection).Inverse();
					const auto nearPoint = MATH::Vector3::EulerTransform({ x * 2 - 1, 1 - y * 2, 0 }, inverse);
					const auto farPoint = MATH::Vector3::EulerTransform({ x * 2 - 1, 1 - y * 2, 1 }, inverse);
					const auto position = gridEnabled ? gridPosition : nearPoint + (farPoint - nearPoint).Normalize() * 5.0f;
					commandList.AddCommand(std::make_unique<AddSavedEntitiesCommand>(sceneManager, QFE::ConvertString(assetPath.wstring()), position));
				} else if (kind == AssetKind::Model) {
					// カメラからドロップ位置を通るレイ上、近平面から5単位の位置に配置する。
					const auto inverse = MATH::Matrix4x4::Multiply(view, projection).Inverse();
					const auto nearPoint = MATH::Vector3::EulerTransform({ x * 2 - 1, 1 - y * 2, 0 }, inverse);
					const auto farPoint = MATH::Vector3::EulerTransform({ x * 2 - 1, 1 - y * 2, 1 }, inverse);
					const auto position = gridEnabled ? gridPosition : nearPoint + (farPoint - nearPoint).Normalize() * 5.0f;
					commandList.AddCommand(std::make_unique<CreateEntityCommand>(ToUtf8(path.stem()), position,
						&manager, relative));
				} else if (kind == AssetKind::Sprite) {
					MATH::Vector3 position{ x * static_cast<float>(width), y * static_cast<float>(height), 0 };
					if (gridEnabled) {
						// Sprite は画面ピクセル座標で描画されるため、格子点を画面へ再投影する。
						const auto clip = MATH::Vector4::EulerTransform(
							{ gridPosition.x, gridPosition.y, gridPosition.z, 1.0f }, MATH::Matrix4x4::Multiply(view, projection));
						if (clip.w > 1.0e-6f) {
							position.x = (clip.x / clip.w + 1.0f) * 0.5f * static_cast<float>(width);
							position.y = (1.0f - clip.y / clip.w) * 0.5f * static_cast<float>(height);
						}
					}
					commandList.AddCommand(std::make_unique<CreateEntityCommand>(ToUtf8(path.stem()), position,
						&manager, std::string{}, true, false, relative));
				}
			}
		}
	}
	ImGui::EndDragDropTarget();
}
