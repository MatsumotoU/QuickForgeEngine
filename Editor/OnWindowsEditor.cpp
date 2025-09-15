#include "OnWindowsEditor.h"
#ifdef _DEBUG
#include "AppUtility/DebugTool/ImGui/FrameController/ImGuiFlameController.h"
#endif // _DEBUG

void OnWindowsEditor::Initialize() {
}

void OnWindowsEditor::Update() {
}

void OnWindowsEditor::Draw() {
#ifdef _DEBUG
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
#endif // _DEBUG
}
