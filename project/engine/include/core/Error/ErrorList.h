#pragma once
#include <string>

namespace QFE {
	// エラーの分類は以下の通り
	// 
	// * exeを落とさないエラー * //
	// UserError: ユーザーの操作や入力に起因するエラー
	// -- PlayerError: プレイヤーの操作に関連するエラー
	// -- DeveloperError: 開発者のコードや設定に起因するエラー
	// 
	// * exeを落とすエラー * //
	// SystemError: システムや環境に起因するエラー
	// -- Abort: プログラムの異常終了を引き起こすエラー
	
	// エラーの種類を定義する列挙型
	enum class ErrorType {
		UserError,
		SystemError
	};
	// ユーザーエラーの種類を定義する列挙型
	enum class UserError {
		PlayerError,
		DeveloperError
	};
	// システムエラーの種類を定義する列挙型
	enum class SystemError {
		Abort
	};

	/// @brief ユーザーエラーを報告する関数
	void ReportUserError(const std::string& message, UserError error);

	/// @brief システムエラーを報告する関数
	void ReportSystemError(const std::string& message, SystemError error);

}
