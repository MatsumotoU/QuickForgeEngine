#pragma once
#include "Utility/SimpleJson.h"

class Asset {
public:
	void SaveMetadata(const std::string& filePath);
	void LoadMetadata(const std::string& filePath);
	
public:
	nlohmann::json GetMetadata() const {
		return metadata_;
	}
	void SetMetadata(const nlohmann::json& metadata) {
		metadata_ = metadata;
	}
	std::string GetName() const {
		return name_;
	}
	void SetName(const std::string& name) {
		name_ = name;
	}

private:
	std::string name_; // アセットの名前
	nlohmann::json metadata_;
};