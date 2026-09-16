#pragma once
#include "DirectoryManager.h"

#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <cstdint>
#include <string>
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
		std::filesystem::path directoryPath;
		PremakeProjectKind kind = PremakeProjectKind::StaticLib;
		ImVec2 initialPosition = ImVec2(0.0f, 0.0f);
		bool positionInitialized = false;
	};

	struct ProjectLink {
		std::uint64_t id = 0;
		std::uint64_t sourceNodeId = 0;
		std::uint64_t targetNodeId = 0;
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
		void DrawDirectoryTree(const std::vector<DirectoryEntry>& directories,
			std::size_t& index, std::uint32_t depth);
		void AddNodeForDirectory(const DirectoryEntry& directory);
		void AddAllDirectoryNodes();
		bool HasNodeForDirectory(const std::filesystem::path& directoryPath) const;
		void RemoveNode(std::uint64_t nodeId);
		void DrawProjectNode(ProjectNode& node);
		void HandleNewLinks();
		void HandleDeletedItems();
		bool TryGetNodeForPin(ax::NodeEditor::PinId pinId,
			std::uint64_t& nodeId, bool& isOutput) const;
		bool LinkExists(std::uint64_t sourceNodeId, std::uint64_t targetNodeId) const;
		bool WouldCreateCycle(std::uint64_t sourceNodeId,
			std::uint64_t targetNodeId) const;
		bool HasPath(std::uint64_t startNodeId, std::uint64_t targetNodeId) const;
		void ArrangeNodesLeftToRight();
		static const char* GetPremakeKindName(PremakeProjectKind kind);

		ImGuiContext imguiContext_;
		DirectoryManager directoryManager_;
		ax::NodeEditor::EditorContext* nodeEditorContext_ = nullptr;
		std::vector<ProjectNode> nodes_;
		std::vector<ProjectLink> links_;
		std::uint64_t nextNodeId_ = 1;
		std::uint64_t nextLinkId_ = 1;
		std::uint64_t pendingNodeRemovalId_ = 0;
	};
}
