#include "ShaderLibReflection.h"
#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

ShaderLibReflection::ShaderLibReflection() {
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	if (FAILED(hr)) {
		QFE_REPORT_SYSTEM_ERROR("Failed to create IDxcUtils instance.", SystemError::Abort);
	}
}

std::vector<RootParameterElement> ShaderLibReflection::GetRootParameterElement(IDxcBlob* libraryBlob) {
	std::vector<RootParameterElement> totalParams;
	if (!libraryBlob || !dxcUtils_) return totalParams;

	// IDxcUtils を使ってライブラリのリフレクションを作成
	DxcBuffer dxcBuffer{};
	dxcBuffer.Ptr = libraryBlob->GetBufferPointer();
	dxcBuffer.Size = libraryBlob->GetBufferSize();
	dxcBuffer.Encoding = DXC_CP_ACP;
	// ID3D12LibraryReflection インターフェースを取得
	Microsoft::WRL::ComPtr<ID3D12LibraryReflection> libraryReflection;
	HRESULT hr = dxcUtils_->CreateReflection(&dxcBuffer, IID_PPV_ARGS(&libraryReflection));

	if (FAILED(hr)) return totalParams;

	// ライブラリ全体の情報を取得
	D3D12_LIBRARY_DESC libraryDesc{};
	libraryReflection->GetDesc(&libraryDesc);

	// 重複バインド（別の関数が同じ定数バッファ等を使っている場合）を弾くための判定ラムダ
	auto isAlreadyAdded = [&](const RootParameterElement& newParam) {
		for (const auto& p : totalParams) {
			// レジスタ番号 と リソースタイプ（CBV/SRV等） が一致していたら重複とみなす
			if (p.shaderRegisterIndex == newParam.shaderRegisterIndex &&
				p.shaderInputType == newParam.shaderInputType) {
				QFE_LOG(std::format(
					"Duplicate resource found: {} at register {}. Skipping.",
					newParam.friendlyName, newParam.shaderRegisterIndex));
				return true;
			}
		}
		return false;
		};

	// 3. ライブラリ内のすべての関数（レイジェネ、ヒットグループなど）をループ
	for (UINT f = 0; f < libraryDesc.FunctionCount; ++f) {
		// 各関数のリフレクションを取得
		ID3D12FunctionReflection* funcReflection = libraryReflection->GetFunctionByIndex(f);
		if (!funcReflection) continue;
		// 各関数の情報を取得
		D3D12_FUNCTION_DESC funcDesc{};
		funcReflection->GetDesc(&funcDesc);
		QFE_LOG(std::format("Function {} has {} bound resources.", funcDesc.Name, funcDesc.BoundResources));

		// 4. 各関数が要求しているバインドリソース（CBV, SRV, UAVなど）をループ
		for (UINT r = 0; r < funcDesc.BoundResources; ++r) {
			D3D12_SHADER_INPUT_BIND_DESC resourceDesc{};
			funcReflection->GetResourceBindingDesc(r, &resourceDesc);

			// RootParameterElement 構造体にデータを詰め替える
			RootParameterElement param{};
			// 前回の寿命問題を考慮し、安全に std::string にディープコピー
			param.friendlyName = resourceDesc.Name;
			param.shaderRegisterIndex = static_cast<int>(resourceDesc.BindPoint);
			param.shaderInputType = resourceDesc.Type;

			// サポート外のタイプ、あるいはすでに他の関数から抽出済みの重複リソースでなければ追加
			if (!isAlreadyAdded(param)) {
				totalParams.push_back(param);
			}

			QFE_LOG(std::format(
				"Resource: {} at register {} added.",
				param.friendlyName, param.shaderRegisterIndex));
		}
	}

	return totalParams;
}