#include "engine/include/collider/ColliderMask.h"
#include "engine/include/utility/String/MyString.h"
#include <nlohmann/json.hpp> 
#include <fstream>

#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

namespace {
	std::string configFilePath = "Resources/Config/ColliderTagMask.json";
}
ColliderTagMask::ColliderTagMask() : tagMaskPairs_{} {
	// コンストラクタ内で未定義動作を避けるためにtry-catchで囲む
	try
	{
		// コンフィグ読み込み処理
		nlohmann::json jsonData;
		std::ifstream ifs(configFilePath);
		if (ifs.is_open()) {
			ifs >> jsonData;
			ifs.close();
		}
		if (jsonData.contains("tagMaskPairs")) {
			for (const auto& pair : jsonData["tagMaskPairs"]) {
				if (pair.contains("tag1") && pair.contains("tag2")) {
					std::string tag1 = pair["tag1"].get<std::string>();
					std::string tag2 = pair["tag2"].get<std::string>();
					tagMaskPairs_.emplace_back(tag1, tag2);
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		e;
#ifdef _DEBUG
		DebugLog(e.what());
#endif // _DEBUG
	}
}

ColliderTagMask::~ColliderTagMask() {
	// デストラクタ内で未定義動作を避けるためにtry-catchで囲む
	try {
		// 保存処理
		nlohmann::json jsonData;
		jsonData["tagMaskPairs"] = nlohmann::json::array();
		for (const auto& pair : tagMaskPairs_) {
			nlohmann::json pairJson;
			pairJson["tag1"] = pair.first;
			pairJson["tag2"] = pair.second;
			jsonData["tagMaskPairs"].push_back(pairJson);
		}
		std::ofstream ofs(configFilePath);
		ofs << jsonData.dump(4);
		ofs.close();
	}catch (const std::exception&e) {
		e;
#ifdef _DEBUG
		DebugLog(e.what());
#endif // _DEBUG
	}
}

void ColliderTagMask::AddTagMaskPair(const std::string& tag1, const std::string& tag2) {
	tagMaskPairs_.emplace_back(tag1, tag2);
}

void ColliderTagMask::EraseTagMaskPair(const std::string& tag1, const std::string& tag2) {
    auto it = std::remove_if(
        tagMaskPairs_.begin(),
        tagMaskPairs_.end(),
        [&](const std::pair<std::string, std::string>& pair) {
            return IsUnorderedPairEqual(pair, std::make_pair(tag1, tag2));
        }
    );
    tagMaskPairs_.erase(it, tagMaskPairs_.end());
}

bool ColliderTagMask::IsCollidable(const std::string& tag1, const std::string& tag2) const {
	for (const auto& pair : tagMaskPairs_) {
		if (IsUnorderedPairEqual(pair, std::make_pair(tag1, tag2))) {
			return true;
		}
	}
	return false;
}

const std::vector<std::pair<std::string, std::string>>& ColliderTagMask::GetTagMaskPairs() const {
	return tagMaskPairs_;
}
