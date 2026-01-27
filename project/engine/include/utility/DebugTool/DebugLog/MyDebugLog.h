#pragma once
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <source_location>
#include <vector>
#include <sol/sol.hpp>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include "engine/include/utility/DesignPatterns/Singleton.h"

namespace QFE {

	enum class LogLevel {
		EngineInfo,
		EditorInfo,
		Warning,
		Error
	};

	/**
	 * ログを出力するクラス.
	 */
	class MyDebugLog : public Singleton<MyDebugLog> {
		friend class Singleton<MyDebugLog>;
	public:
		/// @brief 初期化処理.
		void Initialize();
		/// @brief 終了処理.
		void Finalize();

		/// @brief ログを出力します.
		void Log(const std::string& message, const std::source_location& location = std::source_location::current());
		/// @brief 今までのログを取得します.
		const std::vector<std::string>* GetLog();
		/// @brief 一時的な分類わけされたログを放棄します
		void DebugLogClear();

		// 分類わけされたログ
		std::vector<std::string> engineLog_;
		std::vector<std::string> editorLog_;
		std::vector<std::string> warningLog_;
		std::vector<std::string> errorLog_;

		// Luaスクリプト別ログ
		std::unordered_map<uint32_t, std::unordered_map<std::string, std::vector<std::string>>> scriptLogs_;
	private:
		~MyDebugLog() override = default;

		std::ofstream logStream_;
		std::string logFilePath_;
		std::vector<std::string> log_;

	};

	/// <summary>
	/// 縺薙・繝倥ャ繝€繝ｼ繧定ｪｭ縺ｿ霎ｼ繧薙〒縺・ｌ縺ｰ菴ｿ縺医ｋ繧・▽
	/// </summary>
	/// <param name="message"></param>
	void DebugLog(const std::string& message, const LogLevel& logLevel = LogLevel::EngineInfo, const std::source_location& location = std::source_location::current());

	// Lua縺ｮ蛟､繧呈枚蟄怜・縺ｫ螟画鋤縺励※霑ｽ蜉縺吶ｋ髢｢謨ｰ
	void AppendLuaValueToString(const sol::object& v, std::string& msg);

	// 譁・ｭ怜・繝ｻ謨ｰ蛟､縺ｩ縺｡繧峨ｂ蜿励￠蜿悶ｌ繧九Λ繝・ヱ繝ｼ髢｢謨ｰ
	void DebugLogLua(sol::variadic_args va, uint32_t id, const std::string& scriptName);

	void DebugLogCsharp(const std::string& message);

}
