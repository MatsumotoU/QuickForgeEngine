#include "ProjectGenerator.h"
#include <imgui.h>
#include "core/file/FileUtility.h"
#include "core/string/MyString.h"
#include "framework/window/WindowsWindowFrameWork.h"

#include <algorithm>
#include <cfloat>
#include <unordered_map>
#include <unordered_set>

using namespace QFE::APPLICATION;

namespace
{
	namespace ed = ax::NodeEditor;

	constexpr const char* kPremakeProjectKinds[] = {
		"StaticLib",
		"SharedLib",
		"ConsoleApp",
		"WindowedApp",
		"None",
	};

	ed::NodeId ToEditorNodeId(std::uint64_t id)
	{
		return ed::NodeId(static_cast<std::uintptr_t>(id));
	}

	ed::PinId ToInputPinId(std::uint64_t id)
	{
		return ed::PinId(static_cast<std::uintptr_t>(id * 2 + 1));
	}

	ed::PinId ToOutputPinId(std::uint64_t id)
	{
		return ed::PinId(static_cast<std::uintptr_t>(id * 2 + 2));
	}
}

ProjectGenerator::~ProjectGenerator()
{
	Shutdown();
}

void ProjectGenerator::Initialize(ImGuiContext context) {
	Shutdown();
	imguiContext_ = context;
	directoryManager_.Initialize();

	ax::NodeEditor::Config config;
	// Node positions and links belong to the current project model.  Do not
	// restore a stale canvas viewport from a previous window size.
	config.SettingsFile = nullptr;
	nodeEditorContext_ = ax::NodeEditor::CreateEditor(&config);
	if (nodeEditorContext_ != nullptr) {
		ax::NodeEditor::SetCurrentEditor(nodeEditorContext_);
		auto& style = ax::NodeEditor::GetStyle();
		style.SourceDirection = ImVec2(1.0f, 0.0f);
		style.TargetDirection = ImVec2(-1.0f, 0.0f);
		ax::NodeEditor::SetCurrentEditor(nullptr);
	}
}

void ProjectGenerator::Shutdown()
{
	if (nodeEditorContext_ != nullptr) {
		if (ax::NodeEditor::GetCurrentEditor() == nodeEditorContext_) {
			ax::NodeEditor::SetCurrentEditor(nullptr);
		}
		ax::NodeEditor::DestroyEditor(nodeEditorContext_);
		nodeEditorContext_ = nullptr;
	}
}

void ProjectGenerator::Update() {
	directoryManager_.Update();
}

void ProjectGenerator::Draw() {
    // 1. 画面全体のフラグを設定（タイトルバーやリサイズ、移動などをすべて無効化）
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    // 2. メインビューポート（画面全体）のサイズと位置を取得して、ウィンドウをそこに合わせる
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // 3. ウィンドウのパディング（内側の余白）を一時的にゼロにする（ドックスペースを画面端まで広げるため）
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    // 4. 透明な背景ウィンドウの開始
    ImGui::Begin("MainDockSpaceWindow", nullptr, window_flags);

    // スタイル変数を元に戻す
    ImGui::PopStyleVar(3);

    // 5. ドックスペース（ドッキングの土台）を設置
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyMainDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }

	// メインメニューバーの描画
	MainMenuBar();

    // MainDockSpaceWindowの終了
	ImGui::End();

	// メインウィンドウの描画
	MainWindow();
	// ノードエディタウィンドウの描画
	NodeEditorWindow();
}

void QFE::APPLICATION::ProjectGenerator::MainMenuBar()
{
	ImGui::BeginMenuBar();

    if(ImGui::BeginMenu("File"))
    {
        if(ImGui::MenuItem("Open Loot Directory"))
        {
			// ユーザーにディレクトリを選択させるダイアログを表示
			std::wstring lootDirectory;
			if (QFE::FRAMEWORK::RequestGetDirectoryPathFromUser(
				imguiContext_.hwnd, L"Select Loot Directory", L"*.*", lootDirectory)) {
				if (directoryManager_.SetLootDirectory(QFE::ConvertString(lootDirectory))) {
					nodes_.clear();
					links_.clear();
					nextNodeId_ = 1;
					nextLinkId_ = 1;
				}
			}
        }

        ImGui::EndMenu();
	}
	ImGui::EndMenuBar();
}

void QFE::APPLICATION::ProjectGenerator::MainWindow()
{
	ImGui::SetNextWindowSize(ImVec2(480.0f, 640.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(
		ImVec2(360.0f, 240.0f), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::Begin("Main Window");
	ImGui::Text("Welcome to the Project Generator!");
	ImGui::Text("Loot Directory: %s", directoryManager_.GetLootDirectory().c_str());

	const char* scanState = "Idle";
	switch (directoryManager_.GetScanState()) {
	case DirectoryScanState::Scanning:
		scanState = "Scanning...";
		break;
	case DirectoryScanState::Ready:
		scanState = "Ready";
		break;
	case DirectoryScanState::Failed:
		scanState = "Failed";
		break;
	case DirectoryScanState::Idle:
	default:
		break;
	}
	ImGui::Text("Directory scan: %s", scanState);
	ImGui::Separator();

	const auto& directories = directoryManager_.GetDirectories();
	if (!directories.empty()) {
		if (ImGui::Button("Add All Directories as Nodes")) {
			AddAllDirectoryNodes();
		}
		ImGui::SameLine();
		if (ImGui::Button("Arrange Left to Right")) {
			ArrangeNodesLeftToRight();
		}
	}

	if (ImGui::BeginChild("DirectoryList", ImVec2(0.0f, 0.0f), true)) {
		if (directories.empty()) {
			ImGui::TextUnformatted(
				directoryManager_.GetScanState() == DirectoryScanState::Scanning
					? "Scanning directories..."
					: "No subdirectories found.");
		} else {
			std::size_t index = 0;
			DrawDirectoryTree(directories, index, 0);
		}
	}
	ImGui::EndChild();
	ImGui::End();
}

void QFE::APPLICATION::ProjectGenerator::DrawDirectoryTree(
	const std::vector<DirectoryEntry>& directories,
	std::size_t& index,
	std::uint32_t depth)
{
	while (index < directories.size()) {
		if (directories[index].depth < depth) {
			return;
		}
		if (directories[index].depth > depth) {
			++index;
			continue;
		}

		const DirectoryEntry& directory = directories[index];
		const bool hasChildren = index + 1 < directories.size() &&
			directories[index + 1].depth > depth;
		const std::string relativePath =
			QFE::ConvertString(directory.relativePath.wstring());
		const std::string treeId = "Directory##" + relativePath;
		const ImGuiTreeNodeFlags treeFlags = hasChildren
			? ImGuiTreeNodeFlags_None
			: ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		const bool isOpen = ImGui::TreeNodeEx(
			treeId.c_str(), treeFlags, "%s", directory.name.c_str());

		ImGui::SameLine();
		if (HasNodeForDirectory(directory.absolutePath)) {
			ImGui::TextDisabled("[Added]");
		} else {
			const std::string addButtonId = "Add Node##" + relativePath;
			if (ImGui::SmallButton(addButtonId.c_str())) {
				AddNodeForDirectory(directory);
			}
		}

		++index;
		if (!hasChildren) {
			continue;
		}

		if (isOpen) {
			DrawDirectoryTree(directories, index, depth + 1);
			ImGui::TreePop();
		} else {
			while (index < directories.size() && directories[index].depth > depth) {
				++index;
			}
		}
	}
}

void QFE::APPLICATION::ProjectGenerator::AddNodeForDirectory(
	const DirectoryEntry& directory)
{
	if (HasNodeForDirectory(directory.absolutePath)) {
		return;
	}

	ProjectNode node;
	node.id = nextNodeId_++;
	node.name = directory.name;
	node.directoryPath = directory.absolutePath;
	node.initialPosition = ImVec2(
		static_cast<float>(directory.depth) * 300.0f,
		static_cast<float>(nodes_.size()) * 160.0f);
	nodes_.emplace_back(std::move(node));
}

void QFE::APPLICATION::ProjectGenerator::AddAllDirectoryNodes()
{
	for (const DirectoryEntry& directory : directoryManager_.GetDirectories()) {
		AddNodeForDirectory(directory);
	}
}

bool QFE::APPLICATION::ProjectGenerator::HasNodeForDirectory(
	const std::filesystem::path& directoryPath) const
{
	const std::filesystem::path normalizedPath = directoryPath.lexically_normal();
	return std::any_of(
		nodes_.begin(), nodes_.end(),
		[&normalizedPath](const ProjectNode& node) {
			return node.directoryPath.lexically_normal() == normalizedPath;
		});
}

void QFE::APPLICATION::ProjectGenerator::RemoveNode(std::uint64_t nodeId)
{
	nodes_.erase(
		std::remove_if(
			nodes_.begin(), nodes_.end(),
			[nodeId](const ProjectNode& node) { return node.id == nodeId; }),
		nodes_.end());
	links_.erase(
		std::remove_if(
			links_.begin(), links_.end(),
			[nodeId](const ProjectLink& link) {
				return link.sourceNodeId == nodeId || link.targetNodeId == nodeId;
			}),
		links_.end());
}

void QFE::APPLICATION::ProjectGenerator::NodeEditorWindow()
{
	// Keep the first-use size, but do not force a minimum size.  A dock node can
	// legitimately be smaller than the minimum constraint and ImGui's window
	// clip rect may then remain larger than the actual docked window.
	ImGui::SetNextWindowSize(ImVec2(960.0f, 640.0f), ImGuiCond_FirstUseEver);
	const ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	if (!ImGui::Begin("Node Editor", nullptr, windowFlags)) {
		ImGui::End();
		return;
	}
	if (nodeEditorContext_ == nullptr) {
		ImGui::TextUnformatted("Node editor is not initialized.");
		ImGui::End();
		return;
	}

	// Node Editor 0.9.3 has a ClipRect edge case when its canvas is the only
	// active widget in a window.  Keep a regular ImGui item before the canvas;
	// this also gives the editor a stable content origin after docking/resizing.
	ImGui::TextDisabled("Project Dependency Graph");
	ImGui::Separator();
	const ImVec2 canvasSize = ImGui::GetContentRegionAvail();
	if (canvasSize.x <= 0.0f || canvasSize.y <= 0.0f) {
		ImGui::End();
		return;
	}

	ax::NodeEditor::SetCurrentEditor(nodeEditorContext_);
	ax::NodeEditor::Begin("Project Dependency Graph", canvasSize);

	for (ProjectNode& node : nodes_) {
		const ax::NodeEditor::NodeId editorNodeId = ToEditorNodeId(node.id);
		if (!node.positionInitialized) {
			ax::NodeEditor::SetNodePosition(editorNodeId, node.initialPosition);
			node.positionInitialized = true;
		}

		DrawProjectNode(node);
	}

	for (const ProjectLink& link : links_) {
		// Link() accepts the input pin first, which keeps the dependency
		// direction consistent with QueryNewLink().
		ax::NodeEditor::Link(
			ax::NodeEditor::LinkId(link.id),
			ToInputPinId(link.targetNodeId),
			ToOutputPinId(link.sourceNodeId));
	}

	HandleNewLinks();
	HandleDeletedItems();

	ax::NodeEditor::End();
	ax::NodeEditor::SetCurrentEditor(nullptr);

	if (pendingNodeRemovalId_ != 0) {
		RemoveNode(pendingNodeRemovalId_);
		pendingNodeRemovalId_ = 0;
	}

	ImGui::End();
}

void QFE::APPLICATION::ProjectGenerator::DrawProjectNode(ProjectNode& node)
{
	const ax::NodeEditor::NodeId editorNodeId = ToEditorNodeId(node.id);
	const ax::NodeEditor::PinId inputPinId = ToInputPinId(node.id);
	const ax::NodeEditor::PinId outputPinId = ToOutputPinId(node.id);
	const std::string nodeId = std::to_string(node.id);
	const std::string directoryPath =
		QFE::ConvertString(node.directoryPath.wstring());
	const std::string directoryLabel = "Directory: " + directoryPath;
	const float inputPinWidth = ImGui::CalcTextSize("<- depends on").x;
	const float outputPinWidth = ImGui::CalcTextSize("provides ->").x;
	const float contentWidth = std::max(
		260.0f,
		std::max(
			ImGui::CalcTextSize(node.name.c_str()).x,
			std::max(ImGui::CalcTextSize(directoryLabel.c_str()).x,
				inputPinWidth + outputPinWidth + 64.0f)));

	ax::NodeEditor::BeginNode(editorNodeId);
	ImGui::TextUnformatted(node.name.c_str());
	ImGui::TextUnformatted(directoryLabel.c_str());

	int kind = static_cast<int>(node.kind);
	ImGui::SetNextItemWidth(contentWidth);
	ImGui::Combo(
		("Premake kind##" + nodeId).c_str(),
		&kind,
		kPremakeProjectKinds,
				IM_ARRAYSIZE(kPremakeProjectKinds));
	node.kind = static_cast<PremakeProjectKind>(kind);
	ImGui::TextUnformatted("Language: C++");

	const ImVec2 pinRowStart = ImGui::GetCursorPos();
	const float pinCenterY = pinRowStart.y + ImGui::GetTextLineHeight() * 0.5f;
	const auto& nodeStyle = ax::NodeEditor::GetStyle();
	const ImVec2 inputSocket(
		pinRowStart.x - nodeStyle.NodePadding.x, pinCenterY);
	const float outputStartX = pinRowStart.x + contentWidth - outputPinWidth;
	const ImVec2 outputSocket(
		pinRowStart.x + contentWidth + nodeStyle.NodePadding.z, pinCenterY);

	ax::NodeEditor::BeginPin(inputPinId, ax::NodeEditor::PinKind::Input);
	ax::NodeEditor::PinPivotRect(inputSocket, inputSocket);
	ImGui::TextUnformatted("<- depends on");
	ax::NodeEditor::EndPin();

	ImGui::SameLine(0.0f, 0.0f);
	const float spacerWidth = outputStartX - ImGui::GetCursorPosX();
	if (spacerWidth > 0.0f) {
		ImGui::Dummy(ImVec2(spacerWidth, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
	}

	ax::NodeEditor::BeginPin(outputPinId, ax::NodeEditor::PinKind::Output);
	ax::NodeEditor::PinPivotRect(outputSocket, outputSocket);
	ImGui::TextUnformatted("provides ->");
	ax::NodeEditor::EndPin();

	ImGui::GetWindowDrawList()->AddCircleFilled(
		inputSocket, 5.0f, IM_COL32(80, 180, 255, 255));
	ImGui::GetWindowDrawList()->AddCircleFilled(
		outputSocket, 5.0f, IM_COL32(120, 230, 140, 255));

	if (ImGui::Button(("Delete##" + nodeId).c_str())) {
		pendingNodeRemovalId_ = node.id;
	}
	ax::NodeEditor::EndNode();
}

void QFE::APPLICATION::ProjectGenerator::HandleNewLinks()
{
	if (ax::NodeEditor::BeginCreate()) {
		ax::NodeEditor::PinId firstPinId;
		ax::NodeEditor::PinId secondPinId;
		if (ax::NodeEditor::QueryNewLink(&firstPinId, &secondPinId)) {
			std::uint64_t firstNodeId = 0;
			std::uint64_t secondNodeId = 0;
			bool firstIsOutput = false;
			bool secondIsOutput = false;
			const bool valid =
				TryGetNodeForPin(firstPinId, firstNodeId, firstIsOutput) &&
				TryGetNodeForPin(secondPinId, secondNodeId, secondIsOutput) &&
				firstIsOutput != secondIsOutput;
			const std::uint64_t sourceNodeId = firstIsOutput
				? firstNodeId
				: secondNodeId;
			const std::uint64_t targetNodeId = firstIsOutput
				? secondNodeId
				: firstNodeId;
			const bool validConnection = valid &&
				sourceNodeId != targetNodeId &&
				!LinkExists(sourceNodeId, targetNodeId) &&
				!WouldCreateCycle(sourceNodeId, targetNodeId);

			if (validConnection) {
				if (ax::NodeEditor::AcceptNewItem()) {
					links_.push_back({ nextLinkId_++, sourceNodeId, targetNodeId });
				}
			} else {
				ax::NodeEditor::RejectNewItem();
			}
		}
	}
	ax::NodeEditor::EndCreate();
}

void QFE::APPLICATION::ProjectGenerator::HandleDeletedItems()
{
	if (ax::NodeEditor::BeginDelete()) {
		ax::NodeEditor::LinkId deletedLinkId;
		while (ax::NodeEditor::QueryDeletedLink(&deletedLinkId)) {
			if (ax::NodeEditor::AcceptDeletedItem()) {
				const std::uint64_t linkId =
					static_cast<std::uint64_t>(
						static_cast<std::uintptr_t>(deletedLinkId));
				links_.erase(
					std::remove_if(
						links_.begin(), links_.end(),
						[linkId](const ProjectLink& link) { return link.id == linkId; }),
					links_.end());
			}
		}

		ax::NodeEditor::NodeId deletedNodeId;
		while (ax::NodeEditor::QueryDeletedNode(&deletedNodeId)) {
			if (ax::NodeEditor::AcceptDeletedItem()) {
				pendingNodeRemovalId_ =
					static_cast<std::uint64_t>(
						static_cast<std::uintptr_t>(deletedNodeId));
			}
		}
	}
	ax::NodeEditor::EndDelete();
}

bool QFE::APPLICATION::ProjectGenerator::TryGetNodeForPin(
	ax::NodeEditor::PinId pinId, std::uint64_t& nodeId, bool& isOutput) const
{
	for (const ProjectNode& node : nodes_) {
		if (pinId == ToInputPinId(node.id)) {
			nodeId = node.id;
			isOutput = false;
			return true;
		}
		if (pinId == ToOutputPinId(node.id)) {
			nodeId = node.id;
			isOutput = true;
			return true;
		}
	}
	return false;
}

bool QFE::APPLICATION::ProjectGenerator::LinkExists(
	std::uint64_t sourceNodeId, std::uint64_t targetNodeId) const
{
	return std::any_of(
		links_.begin(), links_.end(),
		[sourceNodeId, targetNodeId](const ProjectLink& link) {
			return link.sourceNodeId == sourceNodeId &&
				link.targetNodeId == targetNodeId;
		});
}

bool QFE::APPLICATION::ProjectGenerator::WouldCreateCycle(
	std::uint64_t sourceNodeId, std::uint64_t targetNodeId) const
{
	return sourceNodeId == targetNodeId || HasPath(targetNodeId, sourceNodeId);
}

bool QFE::APPLICATION::ProjectGenerator::HasPath(
	std::uint64_t startNodeId, std::uint64_t targetNodeId) const
{
	std::vector<std::uint64_t> pendingNodes{ startNodeId };
	std::unordered_set<std::uint64_t> visited;

	while (!pendingNodes.empty()) {
		const std::uint64_t currentNodeId = pendingNodes.back();
		pendingNodes.pop_back();
		if (!visited.insert(currentNodeId).second) {
			continue;
		}
		if (currentNodeId == targetNodeId) {
			return true;
		}

		for (const ProjectLink& link : links_) {
			if (link.sourceNodeId == currentNodeId) {
				pendingNodes.push_back(link.targetNodeId);
			}
		}
	}

	return false;
}

void QFE::APPLICATION::ProjectGenerator::ArrangeNodesLeftToRight()
{
	if (nodeEditorContext_ == nullptr) {
		return;
	}

	std::unordered_map<std::uint64_t, std::uint32_t> levels;
	for (const ProjectNode& node : nodes_) {
		levels[node.id] = 0;
	}

	// A valid graph is acyclic, so repeated relaxation produces a
	// left-to-right topological layout.
	for (std::size_t pass = 0; pass < nodes_.size(); ++pass) {
		for (const ProjectLink& link : links_) {
			levels[link.targetNodeId] = std::max(
				levels[link.targetNodeId], levels[link.sourceNodeId] + 1);
		}
	}

	std::unordered_map<std::uint32_t, std::uint32_t> rows;
	ax::NodeEditor::SetCurrentEditor(nodeEditorContext_);
	for (ProjectNode& node : nodes_) {
		const std::uint32_t level = levels[node.id];
		const std::uint32_t row = rows[level]++;
		node.initialPosition = ImVec2(
			static_cast<float>(level) * 320.0f,
			static_cast<float>(row) * 180.0f);
		node.positionInitialized = true;
		ax::NodeEditor::SetNodePosition(
			ToEditorNodeId(node.id), node.initialPosition);
	}
	ax::NodeEditor::SetCurrentEditor(nullptr);
}

const char* QFE::APPLICATION::ProjectGenerator::GetPremakeKindName(
	PremakeProjectKind kind)
{
	const int index = static_cast<int>(kind);
	if (index < 0 || index >= IM_ARRAYSIZE(kPremakeProjectKinds)) {
		return kPremakeProjectKinds[0];
	}
	return kPremakeProjectKinds[index];
}
