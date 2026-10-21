#include "framework/gui/GuiNodeFramework.h"

#include <cfloat>
#include <cstdint>
#include <utility>

namespace {
	namespace ed = ax::NodeEditor;

	bool IsValidPosition(const ImVec2& position)
	{
		return position.x != FLT_MAX && position.y != FLT_MAX;
	}

	std::vector<ed::NodeId> GetSelectedNodes()
	{
		const int count = ed::GetSelectedNodes(nullptr, 0);
		std::vector<ed::NodeId> result;
		if (count <= 0) {
			return result;
		}
		result.resize(static_cast<std::size_t>(count));
		ed::GetSelectedNodes(result.data(), count);
		return result;
	}

	std::vector<ed::LinkId> GetSelectedLinks()
	{
		const int count = ed::GetSelectedLinks(nullptr, 0);
		std::vector<ed::LinkId> result;
		if (count <= 0) {
			return result;
		}
		result.resize(static_cast<std::size_t>(count));
		ed::GetSelectedLinks(result.data(), count);
		return result;
	}
}

QFE::FRAMEWORK::NodeEditorFramework::~NodeEditorFramework()
{
	Shutdown();
}

void QFE::FRAMEWORK::NodeEditorFramework::Initialize(
	const ax::NodeEditor::Config& config)
{
	Shutdown();
	editorContext_ = ax::NodeEditor::CreateEditor(&config);
}

void QFE::FRAMEWORK::NodeEditorFramework::Initialize()
{
	ax::NodeEditor::Config config;
	config.SettingsFile = nullptr;
	Initialize(config);
}

void QFE::FRAMEWORK::NodeEditorFramework::Shutdown()
{
	if (editorContext_ == nullptr) {
		return;
	}
	if (ax::NodeEditor::GetCurrentEditor() == editorContext_) {
		ax::NodeEditor::SetCurrentEditor(nullptr);
	}
	ax::NodeEditor::DestroyEditor(editorContext_);
	editorContext_ = nullptr;
}

bool QFE::FRAMEWORK::NodeEditorFramework::IsInitialized() const
{
	return editorContext_ != nullptr;
}

void QFE::FRAMEWORK::NodeEditorFramework::SetCallbacks(Callbacks callbacks)
{
	callbacks_ = std::move(callbacks);
}

const QFE::FRAMEWORK::NodeEditorFramework::Callbacks&
QFE::FRAMEWORK::NodeEditorFramework::GetCallbacks() const
{
	return callbacks_;
}

void QFE::FRAMEWORK::NodeEditorFramework::Draw(
	const char* editorId, const ImVec2& canvasSize,
	std::vector<Node>& nodes, const std::vector<Link>& links)
{
	if (editorContext_ == nullptr || editorId == nullptr ||
		canvasSize.x <= 0.0f || canvasSize.y <= 0.0f) {
		return;
	}

	const auto previousEditor = ax::NodeEditor::GetCurrentEditor();
	ax::NodeEditor::SetCurrentEditor(editorContext_);
	ax::NodeEditor::Begin(editorId, canvasSize);

	for (Node& node : nodes) {
		if (node.isGroup && node.groupSize.x > 0.0f &&
			node.groupSize.y > 0.0f) {
			ax::NodeEditor::SetGroupSize(node.id, node.groupSize);
		}
		if (!node.positionInitialized) {
			ax::NodeEditor::SetNodePosition(node.id, node.position);
			node.positionInitialized = true;
		}
		DrawNode(node);
	}

	for (const Link& link : links) {
		ax::NodeEditor::Link(link.id, link.startPinId, link.endPinId,
			link.color, link.thickness);
	}

	PendingEvents pendingEvents;
	HandleNewLinks(pendingEvents);
	HandleDeletedItems(pendingEvents);

	ax::NodeEditor::End();

	for (Node& node : nodes) {
		const ImVec2 position =
			ax::NodeEditor::GetNodePosition(node.id);
		if (IsValidPosition(position)) {
			node.position = position;
		}
	}

	NotifyPendingEvents(pendingEvents);
	NotifySelectionChanged();

	ax::NodeEditor::SetCurrentEditor(previousEditor);
}

void QFE::FRAMEWORK::NodeEditorFramework::DrawNode(Node& node)
{
	ax::NodeEditor::BeginNode(node.id);
	// BeginNode does not create an ImGui ID scope. Keep regular ImGui widgets
	// inside different nodes from colliding when they use the same labels.
	ImGui::PushID(reinterpret_cast<void*>(
		static_cast<std::uintptr_t>(node.id)));
	if (node.draw) {
		node.draw(*this);
	}
	ImGui::PopID();
	ax::NodeEditor::EndNode();
}

void QFE::FRAMEWORK::NodeEditorFramework::HandleNewLinks(
	PendingEvents& pendingEvents)
{
	if (ax::NodeEditor::BeginCreate()) {
		PinId firstPinId;
		PinId secondPinId;
		if (ax::NodeEditor::QueryNewLink(&firstPinId, &secondPinId)) {
			const bool accepted = !callbacks_.canCreateLink ||
				callbacks_.canCreateLink(firstPinId, secondPinId);
			if (accepted) {
				if (ax::NodeEditor::AcceptNewItem()) {
					pendingEvents.createdLinks.push_back({
						firstPinId, secondPinId });
				}
			} else {
				ax::NodeEditor::RejectNewItem();
			}
		}
	}
	ax::NodeEditor::EndCreate();
}

void QFE::FRAMEWORK::NodeEditorFramework::HandleDeletedItems(
	PendingEvents& pendingEvents)
{
	if (ax::NodeEditor::BeginDelete()) {
		LinkId deletedLinkId;
		while (ax::NodeEditor::QueryDeletedLink(&deletedLinkId)) {
			if (ax::NodeEditor::AcceptDeletedItem()) {
				pendingEvents.deletedLinks.push_back(deletedLinkId);
			}
		}

		NodeId deletedNodeId;
		while (ax::NodeEditor::QueryDeletedNode(&deletedNodeId)) {
			if (ax::NodeEditor::AcceptDeletedItem()) {
				pendingEvents.deletedNodes.push_back(deletedNodeId);
			}
		}
	}
	ax::NodeEditor::EndDelete();
}

void QFE::FRAMEWORK::NodeEditorFramework::NotifyPendingEvents(
	const PendingEvents& pendingEvents)
{
	if (callbacks_.onLinkCreated) {
		for (const LinkCreateEvent& event : pendingEvents.createdLinks) {
			callbacks_.onLinkCreated(event.firstPinId, event.secondPinId);
		}
	}
	if (callbacks_.onLinkDeleted) {
		for (const LinkId linkId : pendingEvents.deletedLinks) {
			callbacks_.onLinkDeleted(linkId);
		}
	}
	if (callbacks_.onNodeDeleted) {
		for (const NodeId nodeId : pendingEvents.deletedNodes) {
			callbacks_.onNodeDeleted(nodeId);
		}
	}
}

void QFE::FRAMEWORK::NodeEditorFramework::NotifySelectionChanged()
{
	if (!callbacks_.onSelectionChanged ||
		!ax::NodeEditor::HasSelectionChanged()) {
		return;
	}
	callbacks_.onSelectionChanged(GetSelectedNodes(), GetSelectedLinks());
}

void QFE::FRAMEWORK::NodeEditorFramework::BeginPin(
	PinId pinId, PinKind kind)
{
	ax::NodeEditor::BeginPin(pinId, kind);
}

void QFE::FRAMEWORK::NodeEditorFramework::PinRect(
	const ImVec2& minimum, const ImVec2& maximum)
{
	ax::NodeEditor::PinRect(minimum, maximum);
}

void QFE::FRAMEWORK::NodeEditorFramework::PinPivotRect(
	const ImVec2& minimum, const ImVec2& maximum)
{
	ax::NodeEditor::PinPivotRect(minimum, maximum);
}

void QFE::FRAMEWORK::NodeEditorFramework::PinPivotSize(const ImVec2& size)
{
	ax::NodeEditor::PinPivotSize(size);
}

void QFE::FRAMEWORK::NodeEditorFramework::PinPivotScale(
	const ImVec2& scale)
{
	ax::NodeEditor::PinPivotScale(scale);
}

void QFE::FRAMEWORK::NodeEditorFramework::PinPivotAlignment(
	const ImVec2& alignment)
{
	ax::NodeEditor::PinPivotAlignment(alignment);
}

void QFE::FRAMEWORK::NodeEditorFramework::EndPin()
{
	ax::NodeEditor::EndPin();
}

void QFE::FRAMEWORK::NodeEditorFramework::SetNodePosition(
	NodeId nodeId, const ImVec2& position)
{
	if (editorContext_ == nullptr) {
		return;
	}
	const auto previousEditor = ax::NodeEditor::GetCurrentEditor();
	ax::NodeEditor::SetCurrentEditor(editorContext_);
	ax::NodeEditor::SetNodePosition(nodeId, position);
	ax::NodeEditor::SetCurrentEditor(previousEditor);
}

ImVec2 QFE::FRAMEWORK::NodeEditorFramework::GetNodePosition(NodeId nodeId) const
{
	if (editorContext_ == nullptr) {
		return ImVec2(FLT_MAX, FLT_MAX);
	}
	const auto previousEditor = ax::NodeEditor::GetCurrentEditor();
	ax::NodeEditor::SetCurrentEditor(editorContext_);
	const ImVec2 position = ax::NodeEditor::GetNodePosition(nodeId);
	ax::NodeEditor::SetCurrentEditor(previousEditor);
	return position;
}

ImVec2 QFE::FRAMEWORK::NodeEditorFramework::GetNodeSize(NodeId nodeId) const
{
	if (editorContext_ == nullptr) {
		return ImVec2(0.0f, 0.0f);
	}
	const auto previousEditor = ax::NodeEditor::GetCurrentEditor();
	ax::NodeEditor::SetCurrentEditor(editorContext_);
	const ImVec2 size = ax::NodeEditor::GetNodeSize(nodeId);
	ax::NodeEditor::SetCurrentEditor(previousEditor);
	return size;
}

void QFE::FRAMEWORK::NodeEditorFramework::SetGroupSize(
	NodeId nodeId, const ImVec2& size)
{
	if (editorContext_ == nullptr) {
		return;
	}
	const auto previousEditor = ax::NodeEditor::GetCurrentEditor();
	ax::NodeEditor::SetCurrentEditor(editorContext_);
	ax::NodeEditor::SetGroupSize(nodeId, size);
	ax::NodeEditor::SetCurrentEditor(previousEditor);
}

void QFE::FRAMEWORK::NodeEditorFramework::Group(const ImVec2& size)
{
	ax::NodeEditor::Group(size);
}

void QFE::FRAMEWORK::NodeEditorFramework::ClearSelection()
{
	if (editorContext_ == nullptr) {
		return;
	}
	const auto previousEditor = ax::NodeEditor::GetCurrentEditor();
	ax::NodeEditor::SetCurrentEditor(editorContext_);
	ax::NodeEditor::ClearSelection();
	ax::NodeEditor::SetCurrentEditor(previousEditor);
}

void QFE::FRAMEWORK::NodeEditorFramework::SelectNode(NodeId nodeId)
{
	if (editorContext_ == nullptr) {
		return;
	}
	const auto previousEditor = ax::NodeEditor::GetCurrentEditor();
	ax::NodeEditor::SetCurrentEditor(editorContext_);
	ax::NodeEditor::SelectNode(nodeId);
	ax::NodeEditor::SetCurrentEditor(previousEditor);
}

ImVec2 QFE::FRAMEWORK::NodeEditorFramework::ScreenToCanvas(
	const ImVec2& screenPosition) const
{
	if (editorContext_ == nullptr) {
		return screenPosition;
	}
	const auto previousEditor = ax::NodeEditor::GetCurrentEditor();
	ax::NodeEditor::SetCurrentEditor(editorContext_);
	const ImVec2 canvasPosition = ax::NodeEditor::ScreenToCanvas(screenPosition);
	ax::NodeEditor::SetCurrentEditor(previousEditor);
	return canvasPosition;
}

ImVec4 QFE::FRAMEWORK::NodeEditorFramework::GetNodePadding() const
{
	if (editorContext_ == nullptr) {
		return ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	const auto previousEditor = ax::NodeEditor::GetCurrentEditor();
	ax::NodeEditor::SetCurrentEditor(editorContext_);
	const ImVec4 padding = ax::NodeEditor::GetStyle().NodePadding;
	ax::NodeEditor::SetCurrentEditor(previousEditor);
	return padding;
}

void QFE::FRAMEWORK::NodeEditorFramework::PushStyleColor(
	ax::NodeEditor::StyleColor index, const ImVec4& color)
{
	ax::NodeEditor::PushStyleColor(index, color);
}

void QFE::FRAMEWORK::NodeEditorFramework::PopStyleColor(int count)
{
	ax::NodeEditor::PopStyleColor(count);
}

ax::NodeEditor::Style& QFE::FRAMEWORK::NodeEditorFramework::GetStyle()
{
	static ax::NodeEditor::Style defaultStyle{};
	if (editorContext_ == nullptr) {
		return defaultStyle;
	}
	const auto previousEditor = ax::NodeEditor::GetCurrentEditor();
	ax::NodeEditor::SetCurrentEditor(editorContext_);
	ax::NodeEditor::Style& style = ax::NodeEditor::GetStyle();
	ax::NodeEditor::SetCurrentEditor(previousEditor);
	return style;
}

const ax::NodeEditor::Style&
QFE::FRAMEWORK::NodeEditorFramework::GetStyle() const
{
	static const ax::NodeEditor::Style defaultStyle{};
	if (editorContext_ == nullptr) {
		return defaultStyle;
	}
	const auto previousEditor = ax::NodeEditor::GetCurrentEditor();
	ax::NodeEditor::SetCurrentEditor(editorContext_);
	const ax::NodeEditor::Style& style = ax::NodeEditor::GetStyle();
	ax::NodeEditor::SetCurrentEditor(previousEditor);
	return style;
}

ax::NodeEditor::EditorContext*
QFE::FRAMEWORK::NodeEditorFramework::GetEditorContext() const
{
	return editorContext_;
}
