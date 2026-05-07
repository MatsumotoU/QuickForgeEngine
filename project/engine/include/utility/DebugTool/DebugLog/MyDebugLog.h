#pragma once
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <source_location>
#include <vector>
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

	/// @brief ログを出力する関数. 最適化オフのときのみ有効.
	/// これ自体を呼び出すときは、QFE_LOGマクロを使用してください.ifdefを毎度書く必要がなくなります.
	void DebugLog(const std::string& message, const LogLevel& logLevel = LogLevel::EngineInfo, const std::source_location& location = std::source_location::current());

	/// @brief C#側から呼び出すためのログ出力関数. 最適化オフのときのみ有効.
	void DebugLogCsharp(const std::string& message);

	// 最適化オフのときのみログを出力するマクロ. 最適化オンのときは何もしない.
#ifdef QFE_OPTIMIZE_OFF
#define QFE_LOG(...) DebugLog(__VA_ARGS__)
#else
#define QFE_LOG(...) ((void)0)
#endif

}
