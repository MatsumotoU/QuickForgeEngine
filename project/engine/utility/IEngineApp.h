#pragma once
namespace QFE {
	/// @brief エンジンアプリケーションのインターフェース
	/// アプリケーションはこのインターフェースを実装し、エンジンに渡すことで、エンジンのライフサイクルに合わせた処理を行うことができます。
	class IEngineApp {
	public:
		virtual ~IEngineApp() = default;
		/// @brief アプリケーションの初期化処理
		virtual void Initialize() = 0;
		/// @brief アプリケーションの更新処理
		virtual void Update() = 0;
		/// @brief アプリケーションの描画処理
		virtual void Draw() = 0;
	};
}