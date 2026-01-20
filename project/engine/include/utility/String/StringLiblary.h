#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>

class StringLibrary {
public:
	StringLibrary();
	~StringLibrary();

public:
	/// <summary>
	/// 辞書を�E期化しまぁE
	/// </summary>
	void Init(const std::string& libraryFriendName);

public:
	/// <summary>
	/// 辞書に斁E���Eを登録しまぁE
	/// </summary>
	/// <param name="string"></param>
	void AddStringToLiblary(const std::string& string);
	/// <summary>
	/// 斁E���Eが辞書冁E��あるかどぁE��判定しまぁE
	/// </summary>
	/// <param name="string"></param>
	/// <returns></returns>
	bool FindString(const std::string& string);
	/// <summary>
	/// 持E���E斁E���Eを辞書から探して辞書の添え字を返します。無ぁE��合�E-1を返しまぁE
	/// </summary>
	/// <param name="string"></param>
	/// <returns></returns>
	int32_t GetLiblaryIndex(const std::string& string);
	/// <summary>
	/// チE�Eタの登録名をインチE��クスから探しまぁE
	/// </summary>
	/// <param name="index"></param>
	/// <returns></returns>
	std::string GetDatanameFromIndex(uint32_t index);

private:
	std::string liblaryFriendryName_;
	std::vector<std::string> liblary_;
};
