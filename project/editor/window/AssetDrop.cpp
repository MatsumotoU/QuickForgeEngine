#include "AssetDrop.h"
#include "assetfactory/game-data/GameProjectPaths.h"
#include "command/AddSavedEntitiesCommand.h"
#include "command/CreateEntityCommand.h"
#include "command/EditorCommandList.h"
#include "ecs/entity/EntityManager.h"
#include "scene/SceneManager.h"
#include "string/MyString.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <memory>

namespace {
enum class AssetKind { Unsupported, Entity, Model, Sprite };

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
	const MATH::Matrix4x4& view, const MATH::Matrix4x4& projection) {
	if (!sceneManager || imageSize.x <= 0 || imageSize.y <= 0 || !ImGui::BeginDragDropTarget()) return;
	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(AssetPayloadType)) {
		if (payload->IsDelivery() && payload->DataSize > 1) {
			const auto* bytes = static_cast<const char*>(payload->Data);
			const std::string payloadData(bytes, bytes + payload->DataSize - 1);
			const auto separator = payloadData.find('\n');
			const std::string relative = payloadData.substr(0, separator);
			const auto sourceSeparator = separator == std::string::npos ? std::string::npos : payloadData.find('\n', separator + 1);
			const std::string assetId = separator == std::string::npos ? std::string{} : payloadData.substr(separator + 1, sourceSeparator - separator - 1);
			const auto path = PathFromUtf8(relative);
			const auto assetPath = sourceSeparator == std::string::npos ? PROJECT::Assets() / path :
				PROJECT::Root() / PathFromUtf8(payloadData.substr(sourceSeparator + 1));
			std::error_code error;
			if (bytes[payload->DataSize - 1] == '\0' && !path.is_absolute() &&
				(PROJECT::IsWithin(assetPath, PROJECT::Assets()) ||
					((GetAssetKind(path) == AssetKind::Model || GetAssetKind(path) == AssetKind::Sprite) && PROJECT::IsWithin(assetPath, PROJECT::Root() / "sources"))) &&
				std::filesystem::is_regular_file(assetPath, error) && !error) {
				const AssetKind kind = GetAssetKind(path);
				auto& manager = sceneManager->GetCurrentSceneEntityManager();
				const ImVec2 mouse = ImGui::GetMousePos();
				const float x = (mouse.x - imagePosition.x) / imageSize.x;
				const float y = (mouse.y - imagePosition.y) / imageSize.y;
				if (kind == AssetKind::Entity) {
					commandList.AddCommand(std::make_unique<AddSavedEntitiesCommand>(sceneManager, QFE::ConvertString(assetPath.wstring())));
				} else if (kind == AssetKind::Model) {
					// カメラからドロップ位置を通るレイ上、近平面から5単位の位置に配置する。
					const auto inverse = MATH::Matrix4x4::Multiply(view, projection).Inverse();
					const auto nearPoint = MATH::Vector3::EulerTransform({ x * 2 - 1, 1 - y * 2, 0 }, inverse);
					const auto farPoint = MATH::Vector3::EulerTransform({ x * 2 - 1, 1 - y * 2, 1 }, inverse);
					const auto position = nearPoint + (farPoint - nearPoint).Normalize() * 5.0f;
					commandList.AddCommand(std::make_unique<CreateEntityCommand>(ToUtf8(path.stem()), position,
						&manager, relative, false, false, std::string{}, assetId));
				} else if (kind == AssetKind::Sprite) {
					const MATH::Vector3 position{ x * static_cast<float>(width), y * static_cast<float>(height), 0 };
					commandList.AddCommand(std::make_unique<CreateEntityCommand>(ToUtf8(path.stem()), position,
						&manager, std::string{}, true, false, relative, std::string{}, assetId));
				}
			}
		}
	}
	ImGui::EndDragDropTarget();
}
