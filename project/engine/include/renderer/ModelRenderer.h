/**
 * @file ModelRenderer.h
 * @brief 3Dモデルの描画命令を管理するヘッダー
 */

#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>

/**
 * @namespace Render::Model
 * @brief モデルの描画に関する関数を含む名前空間
 */
namespace Render::Model {
    /**
     * @brief 3Dモデルを描画する
     * @param modelHandle 描画するモデルのハンドル
     */
	void DrawModel(const uint32_t& modelHandle);
}
