#pragma once
#include <string>
#include <memory>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <vector>

#include "math/MathInclude.h"
#include "graphics/D3D12GraphicEngine.h"
#include "assetfactory/model/ModelData.h"

namespace QFE::FRAMEWORK {
	/// @brief Windows版QuickForgeEngineのアプリケーションに必要なリソースを管理するクラス
	class WindowsEngineResources {
	public:
		std::string windowName = "mainWindow";
		uint32_t windowWidth = 1280;
		uint32_t windowHeight = 720;

		std::string assetDir = "resources/";
		std::string modelDir = "resources/";
		std::string psDirName = "engine/resources/shaders/ps/";
		std::string vsDirName = "engine/resources/shaders/vs/";
		std::string rtDirName = "engine/resources/shaders/rt/";

		QFE::GRAPHIC::DirectXResourceHandle globalUVHandle = QFE::GRAPHIC::DirectXResourceHandle::Invalid;
		QFE::GRAPHIC::DirectXResourceHandle globalTriHandle = QFE::GRAPHIC::DirectXResourceHandle::Invalid;
		QFE::GRAPHIC::DirectXResourceHandle instanceMetaHandle = QFE::GRAPHIC::DirectXResourceHandle::Invalid;

		QFE::MATH::EulerTransform cameraTransform;

		std::unordered_map<std::string, QFE::GRAPHIC::DirectXResourceHandle> vertexBufferMap;
		std::unordered_map<std::string, QFE::GRAPHIC::DirectXResourceHandle> indexBufferMap;
		std::unordered_map<std::string, QFE::GRAPHIC::BLASHandle> blasHandleMap;
		std::unordered_map<std::string, QFE::ASSET::ModelData> modelDataMap;

		std::map<std::string, QFE::GRAPHIC::DirectXResourceHandle> textureHandleMap;
		std::map<std::string, uint32_t> textureGpuIndexMap; // テクスチャ名 -> GPU側のインデックスマップ
		uint32_t nextTextureGpuIndex = 2; // 0,1 は BlackCubeMap と White1x1 に予約されているので、次のインデックスは 2 から始める

		QFE::GRAPHIC::ViewPortHandle viewportHandle;
		QFE::GRAPHIC::ScissorRectHandle scissorRectHandle;

		QFE::GRAPHIC::DirectXResourceHandle uavBufferHandle;
		QFE::GRAPHIC::RTPSOHandle rtpsoHandle;

		std::vector<QFE::GRAPHIC::RenderTargetHandle> renderTargets;

		QFE::GRAPHIC::ShaderPairHandle shaderPairHandle;
		QFE::GRAPHIC::PSOHandle psoHandle;
		std::vector<D3D12_ROOT_PARAMETER_TYPE> rootParameterTypes;

		QFE::GRAPHIC::RenderTargetHandle finalRenderTargetHandle = QFE::GRAPHIC::RenderTargetHandle::SwapChain;

		QFE::MATH::Matrix4x4 viewProj = QFE::MATH::Matrix4x4::MakeIdentity4x4();
	};
}