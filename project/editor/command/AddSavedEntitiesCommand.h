#pragma once
#include "IEditorCommand.h"
#include <cstdint>
#include <string>
#include <vector>

namespace QFE::SCENE { class SceneManager; }

namespace QFE::EDITOR {
	/// @brief 保存されたEntity群を現在のシーンへ追加するコマンド。
	class AddSavedEntitiesCommand final : public IEditorCommand {
	public:
		/// @brief 読み込み先のシーンとEntityファイルのUTF-8パスを指定する。
		AddSavedEntitiesCommand(SCENE::SceneManager* scene, std::string path);
		/// @brief 保存されたEntity群を追加する。
		void Execute() override;
		/// @brief このコマンドで追加したEntity群を削除する。
		void Undo() override;

	private:
		SCENE::SceneManager* scene_;
		std::string path_;
		std::vector<uint32_t> created_;
	};
}
