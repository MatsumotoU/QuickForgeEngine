/** 
 * @file OnWindowsEditor.h
 * @brief Windowsプラットフォーム向けエディタの実装
 */

#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include "engine/include/core/IEngineApp.h"
#include "editor/include/UI/UIManager.h"

/** 
 * @class OnWindowsEditor
 * @brief Windowsプラットフォーム向けエディタの実装。ゲームウィンドウとは別にエディタUIを構成する。
*/
namespace QFE {

	class OnWindowsEditor final : public IEngineApp {
	public:
		OnWindowsEditor();
		~OnWindowsEditor() override = default;
		void Initialize() override;
		void Update() override;
		void Draw() override;

	private:
		UIManager uiManager_;
	};
}