/**
 * @file CreateNewProject.h
 * @brief 新しいプロジェクトを作成するためのダイアログUI
 */

#pragma once
#include "../IEditorUI.h"

namespace QFE {
	/**
	 * @class CreateNewProject
	 * @brief 新しいプロジェクトを作成するためのダイアログUI
	 */
	class CreateNewProject : public IEditorUI {
	public:
		void Initialize() override;
		void Update() override;
		void Draw() override;
		void Run() override;
	private:
		std::string projectName_; ///< 新規プロジェクトの名前
	};
}

