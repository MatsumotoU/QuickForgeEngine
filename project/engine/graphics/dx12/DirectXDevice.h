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

/**
 * @class DirectXDevice
 * @brief ID3D12Deviceおよび関連するDXGIインターフェースを取得・生成するクラス
 */
namespace QFE::GRAPHIC::INTERNAL {
	class DirectXDevice final {
	public:
		DirectXDevice();
		~DirectXDevice();
		/** @brief デバイスの初期化 */
		void Initialize();
		/** @brief デバイスの解放 */
		void Shutdown();

		[[nodiscard]] IDXGIFactory7* GetDxgiFactory() const;
		[[nodiscard]] ID3D12Device* GetDevice() const;
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
		Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_;

		bool disableError_;
		bool disableWarning_;
	};
}
