#pragma once
#include <functional>
#include <string>

namespace QFE {
	/// @brief エディタとエンジン間の橋渡しを行うクラス
	class EditorEngineBridge final {
	public:
		EditorEngineBridge() = default;
		~EditorEngineBridge() = default;
		// コピーとムーブを禁止
		EditorEngineBridge(const EditorEngineBridge&) = delete;
		EditorEngineBridge& operator=(const EditorEngineBridge&) = delete;
		EditorEngineBridge(EditorEngineBridge&&) = delete;
		EditorEngineBridge& operator=(EditorEngineBridge&&) = delete;

		// * エディタ側への機能の提供 * //
		// シーンにエンティティを追加する関数群
		static std::function<void()> AddEmptyEntity;
		static std::function<void(const std::string&)> AddEntityFromFile;
		static std::function<void(const std::string&)> AddModelEntity;
		static std::function<void(const std::string&)> AddSpriteEntity;
		static std::function<void(const std::string&, uint32_t)> AddParticleEmitterEntity;
		static std::function<void()> AddCameraEntity;
		// シーンにあるエンティティを操作する関数群
		static std::function<void(uint32_t)> CopyEntity;
		static std::function<void(uint32_t, std::string)> SaveEntity;
		static std::function<void(uint32_t)> DeleteEntity;
		static std::function<void(uint32_t, uint32_t)> ParentChild;

		/// @brief 関数の登録を確認する
		void CheckFunctionRegistration();
		/// @brief 関数の登録を解除する
		void ClearFunctionRegistration();
	};
}