#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>

#include <memory>

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief シェーダーのコンパイルに必要なDXCデバイスを管理するクラス
	class CompilerDevice final {
	public:
		explicit CompilerDevice();
		~CompilerDevice();

	public:
		IDxcUtils* GetDxcUtils() const { return dxcUtils_; }
		IDxcCompiler3* GetDxcCompiler() const { return dxcCompiler_; }
		IDxcIncludeHandler* GetIncludeHandler() const { return includeHandler_; }
	private:

		std::unique_ptr<IDxcUtils, void(*)(IDxcUtils*)> dxcUtils_;
		std::unique_ptr<IDxcCompiler3, void(*)(IDxcCompiler3*)> dxcCompiler_;
		std::unique_ptr<IDxcIncludeHandler, void(*)(IDxcIncludeHandler*)> includeHandler_;
	};
}