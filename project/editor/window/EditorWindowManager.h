#pragma once
#include <string>
#include <memory>
#include <vector>
#include <set>
#include "IEditorWindow.h"

#include <imgui/imgui.h>

namespace QFE {
	class EntityManager;
}

namespace QFE::EDITOR {
	class EditorCommandList;
	

	/// @brief エディタのウィンドウ管理クラス
	class EditorWindowManager final{
	public:
		void Initialize(EntityManager* entityManager, ImTextureID sceneTextureId);
		void Update();
		void Draw(EditorCommandList& commandList);

	private:
		// エディタで表示できるウィンドウたち
		std::vector<std::unique_ptr<IEditorWindow>> editorWindows_;
		std::set<uint32_t> selectedEntities_;
	};
}
