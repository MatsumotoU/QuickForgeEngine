#pragma once
#include <string>
#include <vector>

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
	/// 文字列が辞書内にあるかどうか判定します
	/// </summary>
	/// <param name="string"></param>
	/// <returns></returns>
	bool FindString(const std::string& string);
	/// <summary>
	/// 指定の文字列を辞書から探して辞書の添え字を返します。無い場合は-1を返します
	/// </summary>
	/// <param name="string"></param>
	/// <returns></returns>
	int32_t GetLiblaryIndex(const std::string& string);

private:
	std::string liblaryFriendryName_;
	std::vector<std::string> liblary_;
};