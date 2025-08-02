#pragma once
#include <vector>
#include <string>

class LuaCallFiles {
public:
	LuaCallFiles();

public:
	void CallAssetFile(const std::string& fileName);
	void ClearAssetFiles() {assetFiles_.clear();}
	std::vector<std::string> GetAssetFiles() const {
		return assetFiles_;
	}

private:
	std::vector<std::string> assetFiles_;

};