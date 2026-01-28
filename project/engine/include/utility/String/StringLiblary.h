#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>

namespace QFE {

	class StringLibrary {
	public:
		StringLibrary();
		~StringLibrary();

	public:
		/// <summary>
		/// 辞書をE期化しまぁE
		/// </summary>
		void Init(const std::string& libraryFriendName);

	public:
		/// <summary>
		/// 辞書に斁EEを登録しまぁE
		/// </summary>
		/// <param name="string"></param>
		void AddStringToLiblary(const std::string& string);
		/// <summary>
		/// 斁EEが辞書冁EあるかどぁE判定しまぁE
		/// </summary>
		/// <param name="string"></param>
		/// <returns></returns>
		bool FindString(const std::string& string);
		/// <summary>
		/// 持EE斁EEを辞書から探して辞書の添え字を返します。無ぁE合E-1を返しまぁE
		/// </summary>
		/// <param name="string"></param>
		/// <returns></returns>
		int32_t GetLiblaryIndex(const std::string& string);
		/// <summary>
		/// チEEタの登録名をインチEクスから探しまぁE
		/// </summary>
		/// <param name="index"></param>
		/// <returns></returns>
		std::string GetDatanameFromIndex(uint32_t index);

	private:
		std::string liblaryFriendryName_;
		std::vector<std::string> liblary_;
	};

}
