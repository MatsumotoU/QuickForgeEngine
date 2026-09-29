#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>

class StringLiblary {
public:
	StringLiblary();
	~StringLiblary();

public:
	/// <summary>
	/// 辞書を初期化します
	/// </summary>
	void Init(const std::string& libraryFriendName);

public:
	/// <summary>
	/// 辞書に文字列を登録します
	/// </summary>
	/// <param name="string"></param>
	void AddStringToLiblary(const std::string& string);
	/// <summary>
	/// 文字列が辞書内容あるかどいる判定します
	/// </summary>
	/// <param name="string"></param>
	/// <returns></returns>
	bool FindString(const std::string& string);
	/// <summary>
	/// 指定した文字列を辞書から探して添え字を返す。見つからない場合は-1を返す
	/// </summary>
	/// <param name="string"></param>
	/// <returns></returns>
	int32_t GetLiblaryIndex(const std::string& string);
	/// <summary>
	/// データの登録名をインデックスから探します
	/// </summary>
	/// <param name="index"></param>
	/// <returns></returns>
	std::string GetDatanameFromIndex(uint32_t index);

private:
	std::string liblaryFriendryName_;
	std::vector<std::string> liblary_;
};
