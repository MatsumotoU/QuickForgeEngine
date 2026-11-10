#pragma once
#include "AssimpModelLoader.h"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace QFE::ASSET {
	struct ModelAsset {
		const ModelData* data = nullptr;
		std::shared_ptr<const ModelData> ownedData;
		std::shared_ptr<const AssimpModelLoader> loader;
		std::string sourcePath;
	};

	/// モデルとアニメーションの読み込み結果を所有する。
	class ModelLoader final {
	public:
		void Initialize();
		std::shared_ptr<const ModelAsset> LoadModel(const std::string& name, const std::filesystem::path& modelDirectory, std::string& error);
		const std::unordered_map<std::string, std::shared_ptr<const ModelAsset>>& GetModels() const { return models_; }
	private:
		std::unordered_map<std::string, std::shared_ptr<const ModelAsset>> models_;
	};
}
