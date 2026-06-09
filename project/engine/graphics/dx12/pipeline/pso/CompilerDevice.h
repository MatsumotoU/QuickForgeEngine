#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>

#include <memory>

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief シェーダーのコンパイルに必要なDXCデバイスを管理するクラス
	class CompilerDevice final {
	public:
		/// @brief 初期化、デバイスを生成する
		void Initialize();
		/// @brief 終了、デバイスを解放する
		void Finalize();

	public:
		/// @brief DXCのユーティリティインターフェースを取得する
		IDxcUtils* GetDxcUtils() const;
		/// @brief DXCのコンパイラーインターフェースを取得する
		IDxcCompiler3* GetDxcCompiler() const;
		/// @brief DXCのインクルードハンドラーを取得する
		IDxcIncludeHandler* GetIncludeHandler() const;

	private:
		/// @brief デバイスが生成されているか確認する。生成されていない場合はエラーを報告する
		void CheckDeviceCreated() const;

		bool createdDevice_ = false;

		IDxcUtils* dxcUtils_;
		IDxcCompiler3* dxcCompiler_;
		IDxcIncludeHandler* includeHandler_;
	};
}