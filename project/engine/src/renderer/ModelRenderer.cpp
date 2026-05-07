/**
 * @file ModelRenderer.cpp
 * @brief 3Dモデルのレンダリング機能の実装
 */

#include "engine/include/renderer/ModelRenderer.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/assets/3DModel/Data/ModelRenderData.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include "engine/include/graphic/Pipeline/GraphicPipelineManager.h"
#include <cassert>

#include "Engine/Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

namespace QFE {
	namespace Render {
		namespace Model {

			/**
			 * @brief モデルの描画実行
			 * @param modelHandle 描画するモデルのハンドル
			 */
			void DrawModel(const uint32_t& modelHandle) {
				AssetManager* assetManager = AssetManager::GetInstance();
				assert(assetManager && "AssetManager is nullptr.");

				// TODO: modelHandle の有効性チェック(範囲チェック)が欠けている
				const ModelRenderData* modelDataPtr = assetManager->GetModelRenderData(modelHandle);
				if(modelDataPtr == nullptr) {
#ifdef QFE_OPTIMIZE_OFF
					QFE_LOG(std::string("Error: ModelRenderData is nullptr for modelHandle ") + std::to_string(modelHandle));
#endif // QFE_OPTIMIZE_OFF
					return;
				}
				GpuBufferPool* gpuBufferPool = assetManager->GetGpuBufferPool();

				PipelineStateObject* pso = GraphicPipelineManager::GetInstance()->GetTrianglePso(kBlendModeNormal);

				DirectXCommon* dxCommon = DirectXCommon::GetInstance();
				assert(dxCommon && "DirectXCommon is nullptr.");
				ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT);

				commandList->RSSetViewports(1, dxCommon->GetViewPort());
				commandList->RSSetScissorRects(1, dxCommon->GetScissorRect());
				commandList->SetGraphicsRootSignature(pso->GetRootSignature());
				commandList->SetPipelineState(pso->GetPipelineState());

				// メッシュごとに描画
				for (auto& handle : modelDataPtr->meshRenderDataHandles) {
					commandList->IASetVertexBuffers(0, 1,
						assetManager->GetModelVertexResourceManager()->GetVertexBufferView(handle.vertexBufferHandle));
					commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					commandList->SetGraphicsRootConstantBufferView(0,
						gpuBufferPool->GetConstantBufferAddress<Material>(handle.materialHandle));
					commandList->SetGraphicsRootConstantBufferView(1,
						gpuBufferPool->GetConstantBufferAddress<TransformationMatrix>(handle.wpvBufferHandle));
					commandList->SetGraphicsRootDescriptorTable(2,
						assetManager->GetTextureManager()->GetTextureSrvHandleGPU(handle.textureHandle));
					commandList->SetGraphicsRootConstantBufferView(3,
						gpuBufferPool->GetConstantBufferAddress<DirectionalLight>(handle.lightBufferHandle));
					commandList->SetGraphicsRootConstantBufferView(4,
						gpuBufferPool->GetConstantBufferAddress<CameraForGPU>(handle.cameraPosBufferHandle));
					commandList->DrawInstanced(static_cast<UINT>(
						assetManager->GetModelVertexResourceManager()->GetVertexBufferCount(handle.vertexBufferHandle)), 1, 0, 0);
				}
			}

		}
	}
}
