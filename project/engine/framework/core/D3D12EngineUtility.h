#pragma once
#include "D3D12EngineSystems.h"
#include "QuickForgeSetupConfig.h"
#include <memory>
#include <string>
#include <stdint.h>

namespace QFE::FRAMEWORK {
	/// @brief D3D12用のエンジンシステムをセットアップする関数.設定ファイルのパスを引数に取ります.
	bool SetupEngineSystems(
		D3D12EngineSystems* engineSystems,const QuickForgeSetupConfig& config);
	/// @brief D3D12用のエンジンシステムをシャットダウンする関数.エンジンシステムのポインタを引数に取ります.
	bool ShutdownEngineSystems(D3D12EngineSystems* engineSystems);
	/// @brief フレームの開始処理を行う関数.エンジンシステムのポインタを引数に取ります.
	bool FrameStart(D3D12EngineSystems* engineSystems);
	/// @brief フレームの終了処理を行う関数.エンジンシステムのポインタを引数に取ります.
	bool FrameEnd(D3D12EngineSystems* engineSystems);
	/// @brief 描画前の処理を行う関数.エンジンシステムのポインタを引数に取ります.
	bool PreDraw(D3D12EngineSystems* engineSystems);
	/// @brief 描画後の処理を行う関数.エンジンシステムのポインタを引数に取ります.
	bool PostDraw(D3D12EngineSystems* engineSystems);
}
