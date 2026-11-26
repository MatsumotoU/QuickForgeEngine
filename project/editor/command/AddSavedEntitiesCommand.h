#pragma once
#include "IEditorCommand.h"
#include "math/vector/Vector3.h"
#include <cstdint>
#include <string>
#include <vector>

namespace QFE::SCENE { class SceneManager; }

namespace QFE::EDITOR {
	/// @brief 保存されたEntity群を現在のシーンへ追加するコマンド。
	class AddSavedEntitiesCommand final : public IEditorCommand {
	public:
		/// @brief 読み込み先のシーンとEntityファイルのUTF-8パスを指定する。
		AddSavedEntitiesCommand(SCENE::SceneManager* scene, std::string path,
			QFE::MATH::Vector3 origin = { 0.0f, 0.0f, 0.0f });
		/// @brief 保存されたEntity群を追加する。
		void Execute() override;
		/// @brief このコマンドで追加したEntity群を削除する。
		void Undo() override;

	private:
		SCENE::SceneManager* scene_;
		std::string path_;
		QFE::MATH::Vector3 origin_;
		std::vector<uint32_t> created_;
	};
}
