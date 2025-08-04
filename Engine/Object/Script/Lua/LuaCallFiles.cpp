#include "LuaCallFiles.h"

LuaCallFiles::LuaCallFiles() {
}

void LuaCallFiles::CallAssetFile(const std::string& filename) {
	assetFiles_.push_back(filename);
}
