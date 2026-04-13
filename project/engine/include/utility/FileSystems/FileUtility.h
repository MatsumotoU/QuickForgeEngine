#pragma once
#include <string>
#include <filesystem>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <system_error>
#include <nlohmann/json.hpp>

namespace QFE::FILE {
	/// ある階層のディレクトリ内にある特定拡張子のファイル名一覧を取得します
	extern std::vector<std::string> GetFilesInDirectory(const std::string& directoryPath, const std::string& extension = "");
	/// ある階層にあるディレクトリの一覧を取得します
	extern std::vector<std::string> GetDirectoriesInDirectory(const std::string& directoryPath);
	/// 指定した実行ファイルで特定のファイルを開きます
	extern bool OpenFileOnExe(const std::string& exePath, const std::string& filePath);
	/// 指定したファイルをJSON形式で読み込みます
	extern bool LoadFileToJson(const std::string& filePath, nlohmann::json& json);
	/// ファイル名が特定の拡張子を持っているか確認します
	bool HasExtension(const std::string& fileName, const std::string& extension);
	/// CSVファイルを2次元のuint32_tベクトルに読み込みます
	extern bool LoadCSVToVector(const std::string& filePath, std::vector<std::vector<uint32_t>>& map);
	/// JSON形式のデータを指定したファイルに保存します
	extern bool SaveJSONToFile(const std::string& filePath, const nlohmann::json& json);
	/// UTF-8文字列をワイド文字列に変換します
	extern std::string WideToUTF8(const std::wstring& wstr);
	/// ワイド文字列をUTF-8文字列に変換します
	extern std::wstring GetAbsolutePath(const std::wstring& relativePath);
	/// あるjsonファイルをMsgPack形式で指定のディレクトリに保存します
	extern bool SaveJsonAsMsgPack(const nlohmann::json& jsonFile, const std::string& msgPackSavePath);
	/// MsgPack形式のファイルをjson形式で読み込みます
	extern bool LoadMsgPackToJson(const std::string& msgPackFilePath, nlohmann::json& json);
	/// ディレクトリの存在を確認します
	extern bool HasDirectory(const std::string& directoryPath);
}
