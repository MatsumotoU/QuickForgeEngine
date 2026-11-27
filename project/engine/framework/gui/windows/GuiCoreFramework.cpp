#include "framework/gui/GuiCoreFramework.h"

#include "EngineDefines.h"
#include "window/GameWindowManager.h"
#include "gui/D3D12GuiManager.h"
#include "graphics/D3D12GraphicEngine.h"
#include "graphics/dx12/DirectXDevice.h"
#include "graphics/dx12/vram/descriptors/DescriptorHeapManager.h"
#include "graphics/dx12/command/DirectXCommandManager.h"
#include "graphics/dx12/RenderPass.h"
#include <imgui.h>
#include <cmath>

namespace {
	// 視錐台の外側で線分を切り、画面手前を横切る線が消えたり画面外へ伸びたりしないようにする。
	bool ClipDebugLine(QFE::MATH::Vector4& a, QFE::MATH::Vector4& b) {
		const auto distance = [](const QFE::MATH::Vector4& p, int plane) {
			switch (plane) {
			case 0: return p.w + p.x;
			case 1: return p.w - p.x;
			case 2: return p.w + p.y;
			case 3: return p.w - p.y;
			case 4: return p.z;
			default: return p.w - p.z;
			}
		};
		for (int plane = 0; plane < 6; ++plane) {
			const float da = distance(a, plane), db = distance(b, plane);
			if (!std::isfinite(da) || !std::isfinite(db) || (da < 0.0f && db < 0.0f)) return false;
			if (da < 0.0f) a = a + (b - a) * (da / (da - db));
			else if (db < 0.0f) b = b + (a - b) * (db / (db - da));
		}
		return a.w > 1.0e-6f && b.w > 1.0e-6f;
	}
}

void QFE::FRAMEWORK::DrawDebugLines(const std::vector<DebugLine>& lines,
	const MATH::Matrix4x4& viewProjection, const MATH::Vector2& position,
	const MATH::Vector2& size, bool foreground) {
	if (size.x <= 0.0f || size.y <= 0.0f || lines.empty()) return;
	auto* draw = foreground ? ImGui::GetForegroundDrawList() : ImGui::GetWindowDrawList();
	draw->PushClipRect({ position.x, position.y }, { position.x + size.x, position.y + size.y }, true);
	for (const auto& line : lines) {
		auto a = MATH::Vector4::EulerTransform({ line.start.x, line.start.y, line.start.z, 1.0f }, viewProjection);
		auto b = MATH::Vector4::EulerTransform({ line.end.x, line.end.y, line.end.z, 1.0f }, viewProjection);
		if (!ClipDebugLine(a, b)) continue;
		const auto screen = [&](const MATH::Vector4& p) {
			return ImVec2(position.x + (p.x / p.w + 1.0f) * 0.5f * size.x,
				position.y + (1.0f - p.y / p.w) * 0.5f * size.y);
		};
		const ImU32 color = ImGui::ColorConvertFloat4ToU32({ line.color.x, line.color.y, line.color.z, line.color.w });
		draw->AddLine(screen(a), screen(b), color);
	}
	draw->PopClipRect();
}

std::unique_ptr<QFE::GUI::IGuiManager> QFE::FRAMEWORK::CreateGuiManager(
	const QFE::GRAPHIC::IGraphicEngine& graphicEngine,
	const QFE::WINDOW::IGameWindowManager& windowManager,
	const std::string& windowName) {

	const auto* gameWindowManager =
		dynamic_cast<const QFE::WINDOW::GameWindowManager*>(&windowManager);
	if (gameWindowManager == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("CreateGuiManager: Invalid window manager type", SystemError::Abort);
		return nullptr;
	}

	const HWND hwnd = windowName.empty()
		? gameWindowManager->GetMainWindow()
		: gameWindowManager->GetWindow(windowName);
	if (hwnd == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("CreateGuiManager: Window not found - " + windowName, SystemError::Abort);
		return nullptr;
	}

	const auto* d3d12GraphicEngine =
		dynamic_cast<const QFE::GRAPHIC::D3D12GraphicEngine*>(&graphicEngine);
	if (d3d12GraphicEngine == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("CreateGuiManager: Invalid graphic engine type", SystemError::Abort);
		return nullptr;
	}

	QFE::GRAPHIC::DirectXDevice* device = d3d12GraphicEngine->GetDirectXDevice();
	QFE::GRAPHIC::DirectXCommandManager* commandManager =
		d3d12GraphicEngine->GetDirectXCommandManager();
	QFE::GRAPHIC::DescriptorHeapManager* descriptorHeapManager =
		d3d12GraphicEngine->GetDescriptorHeapManager();
	QFE::GRAPHIC::RenderPass* renderPass = d3d12GraphicEngine->GetRenderPass();
	QFE::GRAPHIC::DescriptorHandles emptySrvHandles =
		descriptorHeapManager->CreateEmptyHeapHandle(QFE::GRAPHIC::DescriptorHeapType::SRV);

	QFE::GUI::D3D12GuiManagerInitDesc guiInitDesc;
	guiInitDesc.device = device->GetDevice();
	guiInitDesc.commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	guiInitDesc.bufferCount = renderPass->GetSwapChainBufferCount();
	guiInitDesc.rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	guiInitDesc.srvHeap = descriptorHeapManager->GetDescriptorHeap(
		QFE::GRAPHIC::DescriptorHeapType::SRV);
	guiInitDesc.cpuHandle = emptySrvHandles.cpuHandle_;
	guiInitDesc.gpuHandle = emptySrvHandles.gpuHandle_;
	guiInitDesc.hwnd = hwnd;

	auto guiManager = std::make_unique<QFE::GUI::D3D12GuiManager>();
	guiManager->Initialize(guiInitDesc);
	return guiManager;
}

void QFE::FRAMEWORK::PreDrawGui(QFE::GUI::IGuiManager& guiManager)
{
	if (auto* manager = dynamic_cast<QFE::GUI::D3D12GuiManager*>(&guiManager)) {
		manager->PreDraw();
	}
}

void QFE::FRAMEWORK::PostDrawGui(QFE::GUI::IGuiManager& guiManager)
{
	if (auto* manager = dynamic_cast<QFE::GUI::D3D12GuiManager*>(&guiManager)) {
		manager->PostDraw();
	}
}

bool QFE::FRAMEWORK::ShutdownGui(QFE::GUI::IGuiManager& guiManager)
{
	if (auto* manager = dynamic_cast<QFE::GUI::D3D12GuiManager*>(&guiManager)) {
		manager->Shutdown();
		return true;
	}
	return false;
}
