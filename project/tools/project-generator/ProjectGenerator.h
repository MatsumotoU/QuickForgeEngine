#pragma once
#include "DirectoryManager.h"

#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
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
		Utility,
	};

	struct ProjectConfigurationSettings {
		PremakeProjectKind kind = PremakeProjectKind::StaticLib;
		std::string includePaths;
		std::string defines;
		std::string preBuildEvent;
		std::string postBuildEvent;
		std::string externalLinks;
		std::string libraryDirectories;
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
		struct ExternalLink {
			std::string filter;
			std::string name;
		};
		struct LibraryDirectory {
			std::string filter;
			std::string path;
		};
		std::vector<ExternalLink> externalLinks;
		std::vector<LibraryDirectory> libraryDirectories;
		std::unordered_map<std::string, ProjectConfigurationSettings>
			configurationSettings;
		ImVec2 initialPosition = ImVec2(0.0f, 0.0f);
		bool positionInitialized = false;
	};

	struct PremakeConfigurationSettings {
		std::string architecture = "x64";
		std::string cppDialect = "C++20";
		bool staticRuntime = true;
		std::string flags = "MultiProcessorCompile";
		std::string buildOptions = "/utf-8";
		std::string debugDirectory =
			"../generated/outputs/%{cfg.buildcfg}/%{cfg.platform}";
		std::string includePaths;
		std::string defines;
		std::string runtime = "Release";
		std::string optimize = "Off";
		std::string symbols = "On";
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

	struct PremakeCommonSettings {
		std::string workspaceName = "QuickForgeEngine";
		std::string architecture = "x64";
		std::string configurations = "Debug\nDevelopment\nRelease";
		std::string cppDialect = "C++20";
		bool staticRuntime = true;
		std::string flags = "MultiProcessorCompile";
		std::string buildOptions = "/utf-8";
		// Premake debugdir path, normally relative to QFE_PROJECT_ROOT.
		// Premake tokens such as %{cfg.buildcfg} are allowed.
		std::string debugDirectory = "../generated/outputs/%{cfg.buildcfg}/%{cfg.platform}";
		std::string includePaths;
		std::string defines;
		std::unordered_map<std::string, PremakeConfigurationSettings>
			configurationSettings;
	};

	class ProjectGenerator {
	public:
		~ProjectGenerator();
		void Initialize(ImGuiContext context);
		void Shutdown();
		void Update();
		void Draw();
		bool GenerateFromConfiguration(
			const std::filesystem::path& configurationPath);

	private:
		void MainMenuBar();
		void MainWindow();
		void SelectRootDirectory();
		void NodeEditorWindow();
		void NodeSettingsWindow();
		void GroupSettingsWindow();
		void CommonPremakeSettingsWindow();
		void EnsureConfigurationData();
		void AddConfiguration(const std::string& name);
		void RemoveConfiguration(const std::string& name);
		void DrawDirectoryTree(const std::vector<DirectoryEntry>& directories,
			std::size_t& index, std::uint32_t depth);
		void AddNodeForDirectory(const DirectoryEntry& directory,
			bool placeAtVisibleCenter = true);
		void AddAllDirectoryNodes();
		bool HasNodeForDirectory(const std::filesystem::path& directoryPath) const;
		ProjectNode* FindNode(std::uint64_t nodeId);
		void RemoveNode(std::uint64_t nodeId);
		void DrawProjectGroup(ProjectGroup& group);
		void DrawProjectNode(ProjectNode& node);
		void DrawNodeContextMenu(
			const std::vector<std::uint64_t>& selectedNodeIds,
			const std::vector<std::uint64_t>& selectedLinkIds,
			const std::vector<std::uint64_t>& selectedGroupIds);
		void DrawCreateProjectInGroupPopup();
		bool CreateDirectoryProjectInGroup(std::uint64_t groupId,
			const std::string& relativeDirectory,
			const std::string& projectName);
		void GroupSelectedNodes(
			const std::vector<std::uint64_t>& selectedNodeIds);
		void DeleteSelectedNodes(
			const std::vector<std::uint64_t>& selectedNodeIds);
		void DeleteSelectedLinks(
			const std::vector<std::uint64_t>& selectedLinkIds);
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
		void LoadConfiguration(
			const std::filesystem::path& configurationPath = {});
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
		bool commonPremakeSettingsOpen_ = false;
		// Cached in NodeEditorWindow so directory-tree actions, which are drawn
		// before the node editor in the same frame, can place new nodes in the
		// currently visible canvas area.
		ImVec2 nodeEditorVisibleCenter_ = ImVec2(0.0f, 0.0f);
		bool nodeEditorVisibleCenterInitialized_ = false;
		std::uint64_t createProjectGroupId_ = 0;
		bool openCreateProjectPopupRequested_ = false;
		std::string newProjectRelativeDirectory_;
		std::string newProjectName_;
		std::string createProjectError_;
		bool directoryRefreshPending_ = false;
		PremakeCommonSettings commonPremakeSettings_;
		std::string premakeStatus_;
	};
}
