#pragma once
#include "nlohmann/json.hpp"
#include <fstream>

namespace SJN {

	const std::string kDirectoryPath = "Resources/GlobalVariables/";

	// この関数たちは外部ライブラリ[nlohmann::json SingleInclude]を使用しています。
	using json = nlohmann::json;

	/// <summary>
	/// jsonデータを読み込みます
	/// </summary>
	/// <param name="fp">ファイルのパス</param>
	/// <returns>指定パスのjsonデータ</returns>
	json LoadJsonData(std::string& fp);

	/// <summary>
	/// jsonデータを外部ファイルに書きだします
	/// </summary>
	/// <param name="fp">jsonグループ名</param>
	/// <param name="saveData">セーブするjsonデータ</param>
	void SaveJsonData(const std::string groupName, json saveData);
}