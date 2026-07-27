/**
 * @file DirectXDevice.h
 * @brief DirectX12デバイスの生成と管理を行うクラス
 */

#pragma once
#include <wrl.h>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>

namespace QFE::GRAPHIC {
	/// @brief DirectX12デバイスの生成と管理を行うクラス
	class DirectXDevice final {
	public:
		DirectXDevice();
		~DirectXDevice();
		/// @brief デバイスの初期化
		void Initialize();
		/// @brief デバイスの解放
		void Shutdown();

		/// @brief IDXGIFactory7を取得する
		[[nodiscard]] IDXGIFactory7* GetDxgiFactory() const;
		/// @brief ID3D12Deviceを取得する
		[[nodiscard]] ID3D12Device* GetDevice() const;
		/// @brief ID3D12Device5を取得する。サポートされていない場合はnullptrを返す。
		[[nodiscard]] ID3D12Device5* GetDevice5() const;
		/// @brief Adapterを取得する。
		[[nodiscard]] IDXGIAdapter4* GetUseAdapter() const;
#ifdef QFE_OPTIMIZE_OFF
		void SetDisableError(bool disable);
		void SetDisableWarning(bool disable);
#endif // QFE_OPTIMIZE_OFF

	private:
		void CreateDxgiFactory();
		void FindAdapter();
		void CreateDevice();

		Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;

		Microsoft::WRL::ComPtr<ID3D12Device> device_;

		bool isDevice5Supported_;
		Microsoft::WRL::ComPtr<ID3D12Device5> device5_;

		Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_;

		bool disableError_;
		bool disableWarning_;
	};
}
