#include "engine/include/collider/ColliderMask.h"
#include "engine/include/utility/String/MyString.h"
#include <nlohmann/json.hpp> 
#include <fstream>

#include "engine/include/core/EngineDefines.h"

using namespace QFE;

void ColliderTagMask::Initialize(const std::string& maskTableFilePath)
{
	// コンフィグファイルパス設定
	configFilePath_ = maskTableFilePath;
	// コンフィグファイル読み込み
	try
	{
		// コンフィグ読み込み処理
		nlohmann::json jsonData;
		std::ifstream ifs(configFilePath_);
		if (ifs.is_open()) {
			ifs >> jsonData;
			ifs.close();
		}
		if (jsonData.contains("tagMaskPairs")) {
			for (const auto& pair : jsonData["tagMaskPairs"]) {
				if (pair.contains("tag1") && pair.contains("tag2")) {
					std::string tag1 = pair["tag1"].get<std::string>();
					std::string tag2 = pair["tag2"].get<std::string>();
					tagMaskPairs_.push_back(std::make_pair(tag1, tag2));
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		e;
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG(e.what());
#endif // QFE_OPTIMIZE_OFF
		// 読み込み失敗時は空のテーブルで初期化
		tagMaskPairs_.clear();
	}
	
}

void ColliderTagMask::Finalize()
{
	try {
		nlohmann::json jsonData;
		jsonData["tagMaskPairs"] = nlohmann::json::array();
		for (const auto& pair : tagMaskPairs_) {
			nlohmann::json pairJson;
			pairJson["tag1"] = pair.first;
			pairJson["tag2"] = pair.second;
			jsonData["tagMaskPairs"].push_back(pairJson);
		}
		std::ofstream ofs(configFilePath_);
		ofs << jsonData.dump(4);
		ofs.close();
	}
	catch (const std::exception& e) {
		e;
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG(e.what());
#endif // QFE_OPTIMIZE_OFF
	}
}

void ColliderTagMask::AddTagMaskPair(const std::string& tag1, const std::string& tag2) {
	tagMaskPairs_.push_back(std::make_pair(tag1, tag2));
}

void ColliderTagMask::EraseTagMaskPair(const std::string& tag1, const std::string& tag2) {
	size_t indexToErase = MAXSIZE_T;
	for (size_t i = 0; i < tagMaskPairs_.size(); ++i) {
		if (IsUnorderedPairEqual(tagMaskPairs_[i], std::make_pair(tag1, tag2))) {
			indexToErase = i;
			break;
		}
	}
    if (indexToErase != MAXSIZE_T) {
        tagMaskPairs_.erase(indexToErase);
	} else {
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG("Tag mask pair not found: (" + tag1 + ", " + tag2 + ")", LogLevel::Error);
#endif // QFE_OPTIMIZE_OFF
	}
}

bool ColliderTagMask::IsCollidable(const std::string& tag1, const std::string& tag2) const {
	for (const auto& pair : tagMaskPairs_) {
		if (IsUnorderedPairEqual(pair, std::make_pair(tag1, tag2))) {
			return true;
		}
	}
	return false;
}

const std::vector<std::pair<std::string, std::string>> ColliderTagMask::GetTagMaskPairs() const {
	std::vector<std::pair<std::string, std::string>> result(tagMaskPairs_.begin(), tagMaskPairs_.end());
	return result;
}
