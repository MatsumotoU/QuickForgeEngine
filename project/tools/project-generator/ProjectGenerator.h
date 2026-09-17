#pragma once
#include "DirectoryManager.h"

#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

#include <imgui_node_editor.h>

namespace QFE::APPLICATION {
	struct ImGuiContext {
		HWND hwnd;
	};

	enum class PremakeProjectKind : std::int32_t {
		StaticLib,
		SharedLib,
		ConsoleApp,
		WindowedApp,
		None,
	};

	struct ProjectNode {
		std::uint64_t id = 0;
		std::string name;
		std::string projectName;
		std::filesystem::path directoryPath;
		PremakeProjectKind kind = PremakeProjectKind::StaticLib;
		std::string includePaths;
		std::string preBuildEvent;
		std::string postBuildEvent;
		ImVec2 initialPosition = ImVec2(0.0f, 0.0f);
		bool positionInitialized = false;
	};

	struct ProjectLink {
		std::uint64_t id = 0;
		std::uint64_t sourceNodeId = 0;
		std::uint64_t targetNodeId = 0;
	};

	struct ProjectGroup {
		std::uint64_t id = 0;
		std::string name;
		std::vector<std::uint64_t> nodeIds;
		ImVec2 initialPosition = ImVec2(0.0f, 0.0f);
		ImVec2 size = ImVec2(420.0f, 260.0f);
		bool positionInitialized = false;
		bool needsBoundsUpdate = false;
	};

	class ProjectGenerator {
	public:
		~ProjectGenerator();
		void Initialize(ImGuiContext context);
		void Shutdown();
		void Update();
		void Draw();

	private:
		void MainMenuBar();
		void MainWindow();
		void NodeEditorWindow();
		void NodeSettingsWindow();
		void GroupSettingsWindow();
		void DrawDirectoryTree(const std::vector<DirectoryEntry>& directories,
			std::size_t& index, std::uint32_t depth);
		void AddNodeForDirectory(const DirectoryEntry& directory);
		void AddAllDirectoryNodes();
		bool HasNodeForDirectory(const std::filesystem::path& directoryPath) const;
		ProjectNode* FindNode(std::uint64_t nodeId);
		void RemoveNode(std::uint64_t nodeId);
		void DrawProjectGroup(ProjectGroup& group);
		void DrawProjectNode(ProjectNode& node);
		void DrawNodeContextMenu(
			const std::vector<std::uint64_t>& selectedNodeIds);
		void GroupSelectedNodes(
			const std::vector<std::uint64_t>& selectedNodeIds);
		void DeleteSelectedNodes(
			const std::vector<std::uint64_t>& selectedNodeIds);
		void UpdateGroupBounds(ProjectGroup& group);
		ProjectGroup* FindGroup(std::uint64_t groupId);
		void RemoveGroup(std::uint64_t groupId);
		void HandleNewLinks();
		void HandleDeletedItems();
		bool TryGetNodeForPin(ax::NodeEditor::PinId pinId,
			std::uint64_t& nodeId, bool& isOutput) const;
		bool LinkExists(std::uint64_t sourceNodeId, std::uint64_t targetNodeId) const;
		bool WouldCreateCycle(std::uint64_t sourceNodeId,
			std::uint64_t targetNodeId) const;
		bool HasPath(std::uint64_t startNodeId, std::uint64_t targetNodeId) const;
		void CollectDependencyClosure(std::uint64_t targetNodeId,
			std::vector<std::uint64_t>& dependencyNodeIds,
			std::unordered_set<std::uint64_t>& visitedNodeIds) const;
		void SimplifyDependencyLinks();
		void ArrangeNodesLeftToRight();
		void LoadRootPremake();
		void GenerateCentralPremake();
		void SaveConfiguration();
		void LoadConfiguration();
		static const char* GetPremakeKindName(PremakeProjectKind kind);

		ImGuiContext imguiContext_;
		DirectoryManager directoryManager_;
		ax::NodeEditor::EditorContext* nodeEditorContext_ = nullptr;
		std::vector<ProjectNode> nodes_;
		std::vector<ProjectLink> links_;
		std::vector<ProjectGroup> groups_;
		std::uint64_t nextNodeId_ = 1;
		std::uint64_t nextLinkId_ = 1;
		std::uint64_t nextGroupId_ = 1;
		std::uint64_t pendingNodeRemovalId_ = 0;
		std::uint64_t selectedNodeId_ = 0;
		std::uint64_t selectedGroupId_ = 0;
		bool nodeSettingsOpen_ = false;
		bool groupSettingsOpen_ = false;
		std::string premakeStatus_;
	};
}
