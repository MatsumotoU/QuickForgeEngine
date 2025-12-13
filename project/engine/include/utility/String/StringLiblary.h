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
	/// 霎樊嶌繧貞・譛溷喧縺励∪縺・
	/// </summary>
	void Init(const std::string& libraryFriendName);

public:
	/// <summary>
	/// 霎樊嶌縺ｫ譁・ｭ怜・繧堤匳骭ｲ縺励∪縺・
	/// </summary>
	/// <param name="string"></param>
	void AddStringToLiblary(const std::string& string);
	/// <summary>
	/// 譁・ｭ怜・縺瑚ｾ樊嶌蜀・↓縺ゅｋ縺九←縺・°蛻､螳壹＠縺ｾ縺・
	/// </summary>
	/// <param name="string"></param>
	/// <returns></returns>
	bool FindString(const std::string& string);
	/// <summary>
	/// 謖・ｮ壹・譁・ｭ怜・繧定ｾ樊嶌縺九ｉ謗｢縺励※霎樊嶌縺ｮ豺ｻ縺亥ｭ励ｒ霑斐＠縺ｾ縺吶ら┌縺・ｴ蜷医・-1繧定ｿ斐＠縺ｾ縺・
	/// </summary>
	/// <param name="string"></param>
	/// <returns></returns>
	int32_t GetLiblaryIndex(const std::string& string);
	/// <summary>
	/// 繝・・繧ｿ縺ｮ逋ｻ骭ｲ蜷阪ｒ繧､繝ｳ繝・ャ繧ｯ繧ｹ縺九ｉ謗｢縺励∪縺・
	/// </summary>
	/// <param name="index"></param>
	/// <returns></returns>
	std::string GetDatanameFromIndex(uint32_t index);

private:
	std::string liblaryFriendryName_;
	std::vector<std::string> liblary_;
};
