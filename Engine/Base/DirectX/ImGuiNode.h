#pragma once
#include <imgui.h>
#include <imgui-node-editor-0.9.3/imgui_node_editor.h>
#include <memory>

class ImGuiNode {
public:
	ImGuiNode();
	~ImGuiNode() = default;

public:
	void Draw();

private:
	ImGuiNode(const ImGuiNode&) = delete;
	ImGuiNode& operator=(const ImGuiNode&) = delete;

	// カスタムデリータ付きunique_ptr
	std::unique_ptr<ax::NodeEditor::EditorContext, void(*)(ax::NodeEditor::EditorContext*)> editorContext_;
};