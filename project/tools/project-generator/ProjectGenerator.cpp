#include "ProjectGenerator.h"
#include <imgui.h>
#include "core/file/FileUtility.h"
#include "core/string/MyString.h"
#include "framework/window/WindowsWindowFrameWork.h"

#include <imgui_stdlib.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cfloat>
#include <cctype>
#include <fstream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string_view>
#include <system_error>
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

	// Node EditorはNodeId/PinId/LinkIdを最終的に同じImGuiのヒットテスト
	// IDとして扱うため、種別ごとに上位ビットを分ける。例えば、単純に
	// nodeId * 2 + 1 とすると、nodeId=3とnodeId=1のinput pinが同じ
	// IDになり、ノードのドラッグがリンク作成として扱われる。
	constexpr std::uintptr_t kNodeEditorIdPayloadMask =
		(std::uintptr_t(1) << 60) - 1;
	constexpr std::uintptr_t kNodeEditorNodeIdTag =
		std::uintptr_t(1) << 60;
	constexpr std::uintptr_t kNodeEditorInputPinIdTag =
		std::uintptr_t(2) << 60;
	constexpr std::uintptr_t kNodeEditorOutputPinIdTag =
		std::uintptr_t(3) << 60;
	constexpr std::uintptr_t kNodeEditorLinkIdTag =
		std::uintptr_t(4) << 60;

	std::uintptr_t EncodeNodeEditorId(
		std::uintptr_t tag, std::uint64_t id)
	{
		return tag | (static_cast<std::uintptr_t>(id) &
			kNodeEditorIdPayloadMask);
	}

	std::uint64_t DecodeNodeEditorNodeId(ed::NodeId id)
	{
		const std::uintptr_t encoded = static_cast<std::uintptr_t>(id);
		if ((encoded & ~kNodeEditorIdPayloadMask) != kNodeEditorNodeIdTag) {
			return 0;
		}
		return static_cast<std::uint64_t>(encoded & kNodeEditorIdPayloadMask);
	}

	ed::NodeId ToEditorNodeId(std::uint64_t id)
	{
		return ed::NodeId(EncodeNodeEditorId(kNodeEditorNodeIdTag, id));
	}

	ed::PinId ToInputPinId(std::uint64_t id)
	{
		return ed::PinId(EncodeNodeEditorId(kNodeEditorInputPinIdTag, id));
	}

	ed::PinId ToOutputPinId(std::uint64_t id)
	{
		return ed::PinId(EncodeNodeEditorId(kNodeEditorOutputPinIdTag, id));
	}

	ed::LinkId ToEditorLinkId(std::uint64_t id)
	{
		return ed::LinkId(EncodeNodeEditorId(kNodeEditorLinkIdTag, id));
	}

	struct ParsedPremakeProject {
		std::string name;
		std::filesystem::path directoryPath;
		PremakeProjectKind kind = PremakeProjectKind::StaticLib;
		std::vector<std::string> includePaths;
		std::vector<std::string> preBuildCommands;
		std::vector<std::string> postBuildCommands;
		std::vector<std::string> links;
	};

	std::string Trim(std::string_view value)
	{
		std::size_t begin = 0;
		while (begin < value.size() &&
			std::isspace(static_cast<unsigned char>(value[begin])) != 0) {
			++begin;
		}

		std::size_t end = value.size();
		while (end > begin &&
			std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
			--end;
		}
		return std::string(value.substr(begin, end - begin));
	}

	bool TryExtractQuotedString(std::string_view value,
		std::size_t searchBegin, std::string& result)
	{
		for (std::size_t index = searchBegin; index < value.size(); ++index) {
			const char quote = value[index];
			if (quote != '"' && quote != '\'') {
				continue;
			}

			std::string parsed;
			for (++index; index < value.size(); ++index) {
				const char character = value[index];
				if (character == '\\' && index + 1 < value.size()) {
					const char escaped = value[++index];
					if (escaped == 'n') {
						parsed.push_back('\n');
					} else {
						parsed.push_back(escaped);
					}
					continue;
				}
				if (character == quote) {
					result = std::move(parsed);
					return true;
				}
				parsed.push_back(character);
			}
			return false;
		}
		return false;
	}

	std::vector<std::string> ExtractQuotedStrings(std::string_view value)
	{
		std::vector<std::string> strings;
		std::size_t searchBegin = 0;
		std::string parsed;
		while (TryExtractQuotedString(value, searchBegin, parsed)) {
			strings.push_back(parsed);
			const std::size_t quoteBegin = value.find_first_of("\"'", searchBegin);
			if (quoteBegin == std::string_view::npos) {
				break;
			}
			const char quote = value[quoteBegin];
			std::size_t quoteEnd = quoteBegin + 1;
			while (quoteEnd < value.size()) {
				if (value[quoteEnd] == '\\') {
					quoteEnd += std::min<std::size_t>(2, value.size() - quoteEnd);
					continue;
				}
				if (value[quoteEnd] == quote) {
					break;
				}
				++quoteEnd;
			}
			searchBegin = quoteEnd < value.size() ? quoteEnd + 1 : value.size();
		}
		return strings;
	}

	bool IsLuaCall(std::string_view line, std::string_view functionName)
	{
		const std::string trimmed = Trim(line);
		if (trimmed.size() <= functionName.size() ||
			trimmed.compare(0, functionName.size(), functionName) != 0) {
			return false;
		}
		const char next = trimmed[functionName.size()];
		return std::isspace(static_cast<unsigned char>(next)) != 0 ||
			next == '(' || next == '{';
	}

	std::string CollectLuaTable(const std::vector<std::string>& lines,
		std::size_t& lineIndex)
	{
		std::string table;
		int braceDepth = 0;
		bool foundOpeningBrace = false;
		bool inString = false;
		char stringQuote = '\0';
		bool escaped = false;

		for (; lineIndex < lines.size(); ++lineIndex) {
			if (!table.empty()) {
				table.push_back('\n');
			}
			table += lines[lineIndex];

			for (const char character : lines[lineIndex]) {
				if (escaped) {
					escaped = false;
					continue;
				}
				if (inString) {
					if (character == '\\') {
						escaped = true;
					} else if (character == stringQuote) {
						inString = false;
					}
					continue;
				}
				if (character == '"' || character == '\'') {
					inString = true;
					stringQuote = character;
				} else if (character == '{') {
					foundOpeningBrace = true;
					++braceDepth;
				} else if (character == '}' && foundOpeningBrace) {
					--braceDepth;
				}
			}

			if (foundOpeningBrace && braceDepth <= 0) {
				break;
			}
		}
		return table;
	}

	PremakeProjectKind ParsePremakeKind(std::string_view value)
	{
		for (std::size_t index = 0; index < IM_ARRAYSIZE(kPremakeProjectKinds); ++index) {
			if (value == kPremakeProjectKinds[index]) {
				return static_cast<PremakeProjectKind>(index);
			}
		}
		return PremakeProjectKind::StaticLib;
	}

	std::vector<ParsedPremakeProject> ParsePremakeProjects(
		const std::filesystem::path& scriptPath, const std::string& source)
	{
		std::vector<std::string> lines;
		std::istringstream sourceStream(source);
		for (std::string line; std::getline(sourceStream, line);) {
			lines.push_back(std::move(line));
		}

		std::vector<ParsedPremakeProject> projects;
		ParsedPremakeProject currentProject;
		bool hasProject = false;
		for (std::size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
			const std::string& line = lines[lineIndex];
			if (IsLuaCall(line, "project")) {
				if (hasProject) {
					projects.push_back(std::move(currentProject));
				}
				currentProject = {};
				currentProject.directoryPath = scriptPath.parent_path();
				TryExtractQuotedString(line, std::string_view("project").size(),
					currentProject.name);
				hasProject = !currentProject.name.empty();
				continue;
			}
			if (!hasProject) {
				continue;
			}

			std::string value;
			if (IsLuaCall(line, "kind") &&
				TryExtractQuotedString(line, std::string_view("kind").size(), value)) {
				currentProject.kind = ParsePremakeKind(value);
				continue;
			}
			if (IsLuaCall(line, "location") &&
				TryExtractQuotedString(line, std::string_view("location").size(), value)) {
				std::filesystem::path location = QFE::ConvertString(value);
				if (!location.is_absolute()) {
					location = scriptPath.parent_path() / location;
				}
				currentProject.directoryPath = location;
				continue;
			}

			const bool isIncludeList = IsLuaCall(line, "includedirs") ||
				IsLuaCall(line, "externalincludedirs");
			const bool isBuildEventList = IsLuaCall(line, "prebuildcommands") ||
				IsLuaCall(line, "postbuildcommands");
			const bool isLinkList = IsLuaCall(line, "links") ||
				IsLuaCall(line, "uses");
			if (!isIncludeList && !isBuildEventList && !isLinkList) {
				continue;
			}

			const std::string table = CollectLuaTable(lines, lineIndex);
			const std::vector<std::string> values = ExtractQuotedStrings(table);
			if (isIncludeList) {
				currentProject.includePaths.insert(
					currentProject.includePaths.end(), values.begin(), values.end());
			} else if (isBuildEventList) {
				auto& destination = IsLuaCall(line, "prebuildcommands")
					? currentProject.preBuildCommands
					: currentProject.postBuildCommands;
				destination.insert(destination.end(), values.begin(), values.end());
			} else {
				currentProject.links.insert(
					currentProject.links.end(), values.begin(), values.end());
			}
		}
		if (hasProject) {
			projects.push_back(std::move(currentProject));
		}
		return projects;
	}

	std::filesystem::path NormalizePath(const std::filesystem::path& path)
	{
		std::error_code error;
		const std::filesystem::path absolutePath =
			std::filesystem::absolute(path, error);
		return (error ? path : absolutePath).lexically_normal();
	}

	std::filesystem::path GetProjectGeneratorDataDirectory()
	{
		std::vector<std::filesystem::path> candidates;
		auto addCandidatesForRoot = [&candidates](const std::filesystem::path& root) {
			candidates.push_back(root / "data");
			candidates.push_back(root / "tools/project-generator/data");
			candidates.push_back(root / "project/tools/project-generator/data");
		};
		auto addAncestors = [&addCandidatesForRoot](std::filesystem::path path) {
			path = NormalizePath(path);
			for (int depth = 0; depth < 8 && !path.empty(); ++depth) {
				addCandidatesForRoot(path);
				const std::filesystem::path parent = path.parent_path();
				if (parent == path) {
					break;
				}
				path = parent;
			}
		};

		std::error_code error;
		addAncestors(std::filesystem::current_path(error));

		// Visual Studioから起動した場合はSolutionDirが作業ディレクトリだが、
		// exeを直接起動した場合はgenerated/outputs/...になるため、両方を探す。
		wchar_t modulePath[MAX_PATH] = {};
		const DWORD modulePathLength = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (modulePathLength != 0) {
			addAncestors(std::filesystem::path(modulePath).parent_path());
		}

		for (const std::filesystem::path& candidate : candidates) {
			error.clear();
			if (std::filesystem::is_directory(candidate, error)) {
				return candidate;
			}
		}

		const std::filesystem::path currentPath =
			NormalizePath(std::filesystem::current_path(error));
		if (currentPath.filename() == "project") {
			return currentPath / "tools/project-generator/data";
		}
		return currentPath / "project/tools/project-generator/data";
	}

	std::filesystem::path GetConfigurationFilePath()
	{
		return GetProjectGeneratorDataDirectory() / "project_generator.json";
	}

	bool TryGetDofilePath(std::string_view line, std::filesystem::path& path)
	{
		const std::size_t dofilePosition = line.find("dofile");
		const std::size_t rootPosition = line.find("_root", dofilePosition);
		if (dofilePosition == std::string_view::npos ||
			rootPosition == std::string_view::npos ||
			line.find("path.join", dofilePosition) == std::string_view::npos) {
			return false;
		}

		std::string includedPath;
		if (!TryExtractQuotedString(line, rootPosition + std::string_view("_root").size(),
			includedPath)) {
			return false;
		}
		path = QFE::ConvertString(includedPath);
		return true;
	}

	void LoadPremakeFileRecursive(const std::filesystem::path& scriptPath,
		std::vector<ParsedPremakeProject>& projects,
		std::unordered_set<std::string>& visitedFiles,
		std::size_t& missingIncludeCount)
	{
		const std::filesystem::path normalizedPath = NormalizePath(scriptPath);
		const std::string visitedKey = normalizedPath.generic_string();
		if (!visitedFiles.insert(visitedKey).second) {
			return;
		}

		std::ifstream input(normalizedPath, std::ios::binary);
		if (!input) {
			++missingIncludeCount;
			return;
		}
		const std::string source(
			(std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

		std::vector<ParsedPremakeProject> parsedProjects =
			ParsePremakeProjects(normalizedPath, source);
		projects.insert(projects.end(),
			std::make_move_iterator(parsedProjects.begin()),
			std::make_move_iterator(parsedProjects.end()));

		std::istringstream sourceStream(source);
		for (std::string line; std::getline(sourceStream, line);) {
			std::filesystem::path includePath;
			if (!TryGetDofilePath(line, includePath)) {
				continue;
			}
			const std::filesystem::path resolvedPath =
				normalizedPath.parent_path() / includePath;
			std::error_code error;
			if (!std::filesystem::is_regular_file(resolvedPath, error)) {
				++missingIncludeCount;
				continue;
			}
			LoadPremakeFileRecursive(resolvedPath, projects, visitedFiles,
				missingIncludeCount);
		}
	}

	std::string JoinLines(const std::vector<std::string>& values)
	{
		std::string result;
		for (const std::string& value : values) {
			const std::string trimmed = Trim(value);
			if (trimmed.empty()) {
				continue;
			}
			if (!result.empty()) {
				result.push_back('\n');
			}
			result += trimmed;
		}
		return result;
	}

	std::vector<std::string> SplitLines(const std::string& value)
	{
		std::vector<std::string> lines;
		std::istringstream stream(value);
		for (std::string line; std::getline(stream, line);) {
			lines.push_back(Trim(line));
		}
		return lines;
	}

	std::string EscapeLuaString(std::string_view value)
	{
		std::string escaped;
		for (const char character : value) {
			switch (character) {
			case '\\':
				escaped += "\\\\";
				break;
			case '"':
				escaped += "\\\"";
				break;
			case '\r':
				break;
			case '\n':
				escaped += "\\n";
				break;
			default:
				escaped.push_back(character);
				break;
			}
		}
		return escaped;
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
	// 選択中ノードの設定ウィンドウ
	NodeSettingsWindow();
}

void QFE::APPLICATION::ProjectGenerator::MainMenuBar()
{
	ImGui::BeginMenuBar();

	    if(ImGui::BeginMenu("File"))
    {
        if(ImGui::MenuItem("Open Root Premake Directory"))
        {
			// ルートpremake5.luaが存在するディレクトリだけをプロジェクトルートにする。
			std::wstring lootDirectory;
			if (QFE::FRAMEWORK::RequestGetDirectoryPathFromUser(
				imguiContext_.hwnd, L"Select a directory containing premake5.lua", L"*.*", lootDirectory)) {
				const std::filesystem::path selectedRoot = QFE::ConvertString(lootDirectory);
				std::error_code error;
				const bool hasRootPremake = std::filesystem::is_regular_file(
					selectedRoot / "premake5.lua", error);
				if (!hasRootPremake) {
					premakeStatus_ = "The selected directory does not contain premake5.lua.";
				} else if (directoryManager_.SetLootDirectory(QFE::ConvertString(lootDirectory))) {
					nodes_.clear();
					links_.clear();
					nextNodeId_ = 1;
					nextLinkId_ = 1;
					selectedNodeId_ = 0;
					nodeSettingsOpen_ = false;
					premakeStatus_.clear();
				} else {
					premakeStatus_ = "Could not start scanning the selected root directory.";
				}
			}
        }

        if (ImGui::MenuItem(
			"Load Root Premake", nullptr, false,
			!directoryManager_.GetLootDirectory().empty())) {
			LoadRootPremake();
		}
		if (ImGui::MenuItem(
			"Generate Central Premake", nullptr, false, !nodes_.empty())) {
			GenerateCentralPremake();
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Save Configuration")) {
			SaveConfiguration();
		}
		if (ImGui::MenuItem("Load Configuration")) {
			LoadConfiguration();
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
	ImGui::Text("Root Directory: %s", directoryManager_.GetLootDirectory().c_str());

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
	if (!premakeStatus_.empty()) {
		ImGui::TextWrapped("Status: %s", premakeStatus_.c_str());
	}
	if (selectedNodeId_ != 0 && ImGui::Button("Open Selected Node Settings")) {
		nodeSettingsOpen_ = true;
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
	node.projectName = directory.name;
	node.directoryPath = directory.absolutePath;
	node.initialPosition = ImVec2(
		static_cast<float>(directory.depth) * 260.0f,
		static_cast<float>(nodes_.size()) * 180.0f);
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

ProjectNode* QFE::APPLICATION::ProjectGenerator::FindNode(std::uint64_t nodeId)
{
	const auto node = std::find_if(
		nodes_.begin(), nodes_.end(),
		[nodeId](const ProjectNode& value) { return value.id == nodeId; });
	return node == nodes_.end() ? nullptr : &(*node);
}

void QFE::APPLICATION::ProjectGenerator::RemoveNode(std::uint64_t nodeId)
{
	if (selectedNodeId_ == nodeId) {
		selectedNodeId_ = 0;
		nodeSettingsOpen_ = false;
	}
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
			ToEditorLinkId(link.id),
			ToInputPinId(link.targetNodeId),
			ToOutputPinId(link.sourceNodeId));
	}

	HandleNewLinks();
	HandleDeletedItems();

	ax::NodeEditor::End();

	// Node Editorの選択状態を設定ウィンドウへ渡す。複数選択時は先頭を
	// 編集対象とし、選択解除時には設定ウィンドウも閉じる。
	ax::NodeEditor::NodeId selectedNodes[1];
	const int selectedNodeCount = ax::NodeEditor::GetSelectedNodes(selectedNodes, 1);
	if (selectedNodeCount > 0) {
		const std::uint64_t selectedNodeId =
			DecodeNodeEditorNodeId(selectedNodes[0]);
		if (FindNode(selectedNodeId) != nullptr && selectedNodeId_ != selectedNodeId) {
			selectedNodeId_ = selectedNodeId;
			nodeSettingsOpen_ = true;
		}
	} else if (selectedNodeId_ != 0) {
		selectedNodeId_ = 0;
		nodeSettingsOpen_ = false;
	}

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
	const std::string projectLabel = node.projectName.empty()
		? node.name
		: node.projectName;
	const float inputPinWidth = ImGui::CalcTextSize("in").x;
	const float outputPinWidth = ImGui::CalcTextSize("out").x;
	const float contentWidth = std::max(
		150.0f,
		std::max(ImGui::CalcTextSize(node.name.c_str()).x,
			std::max(ImGui::CalcTextSize(projectLabel.c_str()).x,
				inputPinWidth + outputPinWidth + 64.0f)));

	ax::NodeEditor::BeginNode(editorNodeId);
	// Node EditorはノードごとにImGuiのIDスコープを作らないため、
	// ノード内の通常のImGuiウィジェットを明示的に分離する。
	ImGui::PushID(reinterpret_cast<void*>(static_cast<std::uintptr_t>(node.id)));
	ImGui::TextUnformatted(node.name.c_str());
	if (projectLabel != node.name) {
		ImGui::TextDisabled("%s", projectLabel.c_str());
	}

	// PinPivotRect()とDrawListは画面座標を受け取る。GetCursorPos()は
	// ウィンドウ内ローカル座標なので、ズーム・ドッキング時に使わない。
	const ImVec2 pinRowStart = ImGui::GetCursorScreenPos();
	const float pinCenterY = pinRowStart.y + ImGui::GetTextLineHeight() * 0.5f;
	const auto& nodeStyle = ax::NodeEditor::GetStyle();
	const ImVec2 inputSocket(
		pinRowStart.x - nodeStyle.NodePadding.x, pinCenterY);
	const float outputStartX = pinRowStart.x + contentWidth - outputPinWidth;
	const ImVec2 outputSocket(
		pinRowStart.x + contentWidth + nodeStyle.NodePadding.z, pinCenterY);

	ax::NodeEditor::BeginPin(inputPinId, ax::NodeEditor::PinKind::Input);
	ax::NodeEditor::PinPivotRect(inputSocket, inputSocket);
	ImGui::TextUnformatted("in");
	ax::NodeEditor::EndPin();

	ImGui::SameLine(0.0f, 0.0f);
	const float spacerWidth = outputStartX - ImGui::GetCursorScreenPos().x;
	if (spacerWidth > 0.0f) {
		ImGui::Dummy(ImVec2(spacerWidth, 0.0f));
		ImGui::SameLine(0.0f, 0.0f);
	}

	ax::NodeEditor::BeginPin(outputPinId, ax::NodeEditor::PinKind::Output);
	ax::NodeEditor::PinPivotRect(outputSocket, outputSocket);
	ImGui::TextUnformatted("out");
	ax::NodeEditor::EndPin();

	ImGui::GetWindowDrawList()->AddCircleFilled(
		inputSocket, 5.0f, IM_COL32(80, 180, 255, 255));
	ImGui::GetWindowDrawList()->AddCircleFilled(
		outputSocket, 5.0f, IM_COL32(120, 230, 140, 255));

	if (ImGui::Button("Delete")) {
		pendingNodeRemovalId_ = node.id;
	}
	ax::NodeEditor::EndNode();
	ImGui::PopID();
}

void QFE::APPLICATION::ProjectGenerator::NodeSettingsWindow()
{
	if (!nodeSettingsOpen_ || selectedNodeId_ == 0) {
		return;
	}

	ProjectNode* node = FindNode(selectedNodeId_);
	if (node == nullptr) {
		selectedNodeId_ = 0;
		nodeSettingsOpen_ = false;
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(420.0f, 560.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(
		ImVec2(320.0f, 320.0f), ImVec2(FLT_MAX, FLT_MAX));
	bool isOpen = nodeSettingsOpen_;
	if (ImGui::Begin("Node Settings", &isOpen)) {
		ImGui::Text("Directory node: %s", node->name.c_str());
		const std::string directoryPath =
			QFE::ConvertString(node->directoryPath.wstring());
		ImGui::TextWrapped("Path: %s", directoryPath.c_str());
		ImGui::Separator();

		ImGui::TextUnformatted("Premake project name");
		ImGui::InputText("##PremakeProjectName", &node->projectName);

		int kind = static_cast<int>(node->kind);
		ImGui::TextUnformatted("Premake kind");
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::Combo("##PremakeKind", &kind, kPremakeProjectKinds,
			IM_ARRAYSIZE(kPremakeProjectKinds))) {
			node->kind = static_cast<PremakeProjectKind>(kind);
		}

		ImGui::TextUnformatted("Include paths (one path per line)");
		ImGui::InputTextMultiline(
			"##NodeIncludePaths", &node->includePaths,
			ImVec2(-FLT_MIN, 96.0f), ImGuiInputTextFlags_AllowTabInput);

		ImGui::TextUnformatted("Pre-build event (one command per line)");
		ImGui::InputTextMultiline(
			"##NodePreBuildEvent", &node->preBuildEvent,
			ImVec2(-FLT_MIN, 88.0f), ImGuiInputTextFlags_AllowTabInput);

		ImGui::TextUnformatted("Post-build event (one command per line)");
		ImGui::InputTextMultiline(
			"##NodePostBuildEvent", &node->postBuildEvent,
			ImVec2(-FLT_MIN, 88.0f), ImGuiInputTextFlags_AllowTabInput);
	}
	ImGui::End();

	if (!isOpen) {
		nodeSettingsOpen_ = false;
		selectedNodeId_ = 0;
		if (nodeEditorContext_ != nullptr) {
			ax::NodeEditor::SetCurrentEditor(nodeEditorContext_);
			ax::NodeEditor::ClearSelection();
			ax::NodeEditor::SetCurrentEditor(nullptr);
		}
	}
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
				const std::uintptr_t encodedLinkId =
					static_cast<std::uintptr_t>(deletedLinkId);
				const std::uint64_t linkId =
					(encodedLinkId & ~kNodeEditorIdPayloadMask) == kNodeEditorLinkIdTag
					? static_cast<std::uint64_t>(
						encodedLinkId & kNodeEditorIdPayloadMask)
					: 0;
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
				pendingNodeRemovalId_ = DecodeNodeEditorNodeId(deletedNodeId);
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

void QFE::APPLICATION::ProjectGenerator::LoadRootPremake()
{
	if (directoryManager_.GetLootDirectory().empty()) {
		premakeStatus_ = "Select a root directory first.";
		return;
	}
	if (directoryManager_.GetScanState() == DirectoryScanState::Scanning) {
		premakeStatus_ = "Wait for the root directory scan to finish.";
		return;
	}

	const std::filesystem::path rootPath =
		NormalizePath(QFE::ConvertString(directoryManager_.GetLootDirectory()));
	const std::filesystem::path rootPremakePath = rootPath / "premake5.lua";
	std::error_code fileError;
	if (!std::filesystem::is_regular_file(rootPremakePath, fileError)) {
		premakeStatus_ = "Root premake5.lua was not found.";
		return;
	}

	std::vector<ParsedPremakeProject> projects;
	std::unordered_set<std::string> visitedFiles;
	std::size_t missingIncludeCount = 0;
	LoadPremakeFileRecursive(rootPremakePath, projects, visitedFiles,
		missingIncludeCount);
	if (projects.empty()) {
		premakeStatus_ = "No project declarations were found in the root Premake scripts.";
		return;
	}

	// ルートから参照される各スクリプトの location を、走査済みの
	// ディレクトリノードへ対応付ける。未追加のプロジェクトはここで追加する。
	for (const ParsedPremakeProject& project : projects) {
		const std::filesystem::path projectPath = NormalizePath(project.directoryPath);
		for (const DirectoryEntry& directory : directoryManager_.GetDirectories()) {
			if (NormalizePath(directory.absolutePath) == projectPath) {
				AddNodeForDirectory(directory);
				break;
			}
		}
	}

	std::unordered_map<std::string, std::uint64_t> nodeIdsByPath;
	for (const ProjectNode& node : nodes_) {
		nodeIdsByPath[NormalizePath(node.directoryPath).generic_string()] = node.id;
	}

	std::unordered_set<std::string> appliedProjectPaths;
	std::size_t matchedProjectCount = 0;
	for (const ParsedPremakeProject& project : projects) {
		const std::string pathKey = NormalizePath(project.directoryPath).generic_string();
		const auto nodeId = nodeIdsByPath.find(pathKey);
		if (nodeId == nodeIdsByPath.end() ||
			!appliedProjectPaths.insert(pathKey).second) {
			continue;
		}

		ProjectNode* node = FindNode(nodeId->second);
		if (node == nullptr) {
			continue;
		}
		node->projectName = project.name;
		node->kind = project.kind;
		node->includePaths = JoinLines(project.includePaths);
		node->preBuildEvent = JoinLines(project.preBuildCommands);
		node->postBuildEvent = JoinLines(project.postBuildCommands);
		++matchedProjectCount;
	}

	// project.links { "Provider" } は「このプロジェクトが Provider に依存」
	// という意味なので、Provider の出力ピンから対象プロジェクトの入力ピンへ
	// エッジを作る。
	std::unordered_map<std::string, std::uint64_t> nodeIdsByProjectName;
	for (const ProjectNode& node : nodes_) {
		const std::string projectName = node.projectName.empty()
			? node.name
			: node.projectName;
		nodeIdsByProjectName[projectName] = node.id;
	}

	links_.clear();
	nextLinkId_ = 1;
	std::size_t importedLinkCount = 0;
	for (const ParsedPremakeProject& project : projects) {
		const auto target = nodeIdsByProjectName.find(project.name);
		if (target == nodeIdsByProjectName.end()) {
			continue;
		}
		for (const std::string& dependencyName : project.links) {
			const auto source = nodeIdsByProjectName.find(dependencyName);
			if (source == nodeIdsByProjectName.end() ||
				source->second == target->second ||
				LinkExists(source->second, target->second) ||
				WouldCreateCycle(source->second, target->second)) {
				continue;
			}
			links_.push_back({ nextLinkId_++, source->second, target->second });
			++importedLinkCount;
		}
	}

	std::ostringstream status;
	status << "Loaded " << visitedFiles.size() << " Premake file(s), matched "
		<< matchedProjectCount << "/" << projects.size() << " project(s), imported "
		<< importedLinkCount << " link(s).";
	if (missingIncludeCount != 0) {
		status << " Missing include(s): " << missingIncludeCount << ".";
	}
	premakeStatus_ = status.str();
}

void QFE::APPLICATION::ProjectGenerator::GenerateCentralPremake()
{
	if (directoryManager_.GetLootDirectory().empty()) {
		premakeStatus_ = "Select a root directory first.";
		return;
	}
	if (nodes_.empty()) {
		premakeStatus_ = "Add at least one node before generating Premake.";
		return;
	}

	const std::filesystem::path rootPath =
		NormalizePath(QFE::ConvertString(directoryManager_.GetLootDirectory()));
	const std::filesystem::path outputPath = rootPath / "premake5.generated.lua";
	std::ofstream output(outputPath, std::ios::trunc);
	if (!output) {
		premakeStatus_ = "Could not write premake5.generated.lua.";
		return;
	}

	std::unordered_map<std::uint64_t, std::string> generatedNames;
	std::unordered_set<std::string> usedNames;
	for (const ProjectNode& node : nodes_) {
		std::string projectName = node.projectName.empty() ? node.name : node.projectName;
		if (!usedNames.insert(projectName).second) {
			projectName += "_" + std::to_string(node.id);
			usedNames.insert(projectName);
		}
		generatedNames[node.id] = std::move(projectName);
	}

	auto relativeDirectory = [&rootPath](const std::filesystem::path& directory) {
		std::error_code error;
		std::filesystem::path relative =
			std::filesystem::relative(NormalizePath(directory), rootPath, error);
		if (error || relative.empty()) {
			return std::string(".");
		}
		return relative.generic_string();
	};
	auto writeStringList = [&output](const char* setting,
		const std::vector<std::string>& values) {
		std::vector<std::string> nonEmptyValues;
		for (const std::string& value : values) {
			if (!Trim(value).empty()) {
				nonEmptyValues.push_back(Trim(value));
			}
		}
		if (nonEmptyValues.empty()) {
			return;
		}
		output << "    " << setting << " {\n";
		for (const std::string& value : nonEmptyValues) {
			output << "        \"" << EscapeLuaString(value) << "\",\n";
		}
		output << "    }\n";
	};

	output << "-- Generated by QuickForgeEngine ProjectGenerator.\n";
	output << "-- This file keeps all project declarations in one place.\n\n";
	output << "workspace \"QuickForgeEngine\"\n";
	output << "    architecture \"x64\"\n";
	output << "    configurations { \"Debug\", \"Development\", \"Release\" }\n";
	output << "    cppdialect \"C++20\"\n";
	output << "    staticruntime \"on\"\n";
	output << "    flags { \"MultiProcessorCompile\" }\n";
	output << "    objdir (\"../generated/obj/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}\")\n";
	output << "    targetdir (\"../generated/outputs/%{cfg.buildcfg}/%{cfg.platform}\")\n\n";

	for (const ProjectNode& node : nodes_) {
		const std::string& projectName = generatedNames[node.id];
		const std::string relativePath = relativeDirectory(node.directoryPath);
		const std::string fileRoot = relativePath == "."
			? std::string()
			: relativePath + "/";

		output << "project \"" << EscapeLuaString(projectName) << "\"\n";
		output << "    location \"" << EscapeLuaString(relativePath) << "\"\n";
		output << "    kind \"" << EscapeLuaString(GetPremakeKindName(node.kind)) << "\"\n";
		output << "    language \"C++\"\n";
		output << "    files {\n";
		output << "        \"" << EscapeLuaString(fileRoot + "**.h") << "\",\n";
		output << "        \"" << EscapeLuaString(fileRoot + "**.cpp") << "\",\n";
		output << "    }\n";

		writeStringList("includedirs", SplitLines(node.includePaths));
		std::vector<std::string> nodeLinks;
		for (const ProjectLink& link : links_) {
			if (link.targetNodeId != node.id) {
				continue;
			}
			const auto sourceName = generatedNames.find(link.sourceNodeId);
			if (sourceName != generatedNames.end()) {
				nodeLinks.push_back(sourceName->second);
			}
		}
		writeStringList("links", nodeLinks);
		writeStringList("prebuildcommands", SplitLines(node.preBuildEvent));
		writeStringList("postbuildcommands", SplitLines(node.postBuildEvent));
		output << "\n";
	}

	output.flush();
	if (!output) {
		premakeStatus_ = "Failed while writing premake5.generated.lua.";
		return;
	}
	premakeStatus_ = "Generated central Premake: " +
		QFE::ConvertString(outputPath.wstring());
}

void QFE::APPLICATION::ProjectGenerator::SaveConfiguration()
{
	const std::filesystem::path configurationPath = GetConfigurationFilePath();
	std::error_code error;
	std::filesystem::create_directories(configurationPath.parent_path(), error);
	if (error) {
		premakeStatus_ = "Could not create the ProjectGenerator data directory.";
		return;
	}

	nlohmann::json configuration = nlohmann::json::object();
	configuration["version"] = 1;
	configuration["rootDirectory"] = directoryManager_.GetLootDirectory();
	configuration["nextNodeId"] = nextNodeId_;
	configuration["nextLinkId"] = nextLinkId_;
	configuration["nodes"] = nlohmann::json::array();
	if (nodeEditorContext_ != nullptr) {
		ax::NodeEditor::SetCurrentEditor(nodeEditorContext_);
	}
	for (const ProjectNode& node : nodes_) {
		const ImVec2 currentPosition = nodeEditorContext_ != nullptr
			? ax::NodeEditor::GetNodePosition(ToEditorNodeId(node.id))
			: node.initialPosition;
		nlohmann::json nodeJson = nlohmann::json::object();
		nodeJson["id"] = node.id;
		nodeJson["name"] = node.name;
		nodeJson["projectName"] = node.projectName;
		nodeJson["directoryPath"] =
			QFE::ConvertString(node.directoryPath.wstring());
		nodeJson["kind"] = static_cast<int>(node.kind);
		nodeJson["includePaths"] = node.includePaths;
		nodeJson["preBuildEvent"] = node.preBuildEvent;
		nodeJson["postBuildEvent"] = node.postBuildEvent;
		nodeJson["position"] = {
			{ "x", currentPosition.x },
			{ "y", currentPosition.y },
		};
		configuration["nodes"].push_back(std::move(nodeJson));
	}
	if (nodeEditorContext_ != nullptr) {
		ax::NodeEditor::SetCurrentEditor(nullptr);
	}

	configuration["links"] = nlohmann::json::array();
	for (const ProjectLink& link : links_) {
		configuration["links"].push_back({
			{ "id", link.id },
			{ "sourceNodeId", link.sourceNodeId },
			{ "targetNodeId", link.targetNodeId },
		});
	}

	const std::string configurationPathString =
		QFE::ConvertString(configurationPath.wstring());
	if (!QFE::FILE::SaveJSONToFile(configurationPathString, configuration)) {
		premakeStatus_ = "Could not save ProjectGenerator configuration.";
		return;
	}
	premakeStatus_ = "Saved configuration: " + configurationPathString;
}

void QFE::APPLICATION::ProjectGenerator::LoadConfiguration()
{
	const std::filesystem::path configurationPath = GetConfigurationFilePath();
	std::ifstream input(configurationPath);
	if (!input) {
		premakeStatus_ = "ProjectGenerator configuration was not found.";
		return;
	}

	nlohmann::json configuration;
	try {
		input >> configuration;
	} catch (const nlohmann::json::exception&) {
		premakeStatus_ = "ProjectGenerator configuration contains invalid JSON.";
		return;
	}
	if (!configuration.is_object() ||
		!configuration.contains("nodes") || !configuration["nodes"].is_array()) {
		premakeStatus_ = "ProjectGenerator configuration has an invalid format.";
		return;
	}

	std::string rootLoadStatus;
	const std::string rootDirectory = configuration.value("rootDirectory", "");
	if (!rootDirectory.empty()) {
		const std::filesystem::path rootPath = QFE::ConvertString(rootDirectory);
		std::error_code error;
		const bool hasRootPremake = std::filesystem::is_regular_file(
			rootPath / "premake5.lua", error);
		if (!std::filesystem::is_directory(rootPath, error) || !hasRootPremake) {
			rootLoadStatus = " Saved root does not contain premake5.lua.";
		} else if (!directoryManager_.SetLootDirectory(rootDirectory)) {
			rootLoadStatus = " Could not start the saved root directory scan.";
		}
	}

	nodes_.clear();
	links_.clear();
	nextNodeId_ = configuration.value("nextNodeId", std::uint64_t(1));
	nextLinkId_ = configuration.value("nextLinkId", std::uint64_t(1));
	if (nextNodeId_ == 0) {
		nextNodeId_ = 1;
	}
	if (nextLinkId_ == 0) {
		nextLinkId_ = 1;
	}
	selectedNodeId_ = 0;
	nodeSettingsOpen_ = false;

	std::unordered_set<std::uint64_t> loadedNodeIds;
	std::size_t invalidNodeCount = 0;
	for (const nlohmann::json& nodeJson : configuration["nodes"]) {
		try {
			const std::uint64_t nodeId = nodeJson.value("id", std::uint64_t(0));
			const std::string directoryPath = nodeJson.value("directoryPath", "");
			if (nodeId == 0 || directoryPath.empty() ||
				!loadedNodeIds.insert(nodeId).second) {
				++invalidNodeCount;
				continue;
			}

			ProjectNode node;
			node.id = nodeId;
			node.name = nodeJson.value("name", "");
			node.projectName = nodeJson.value("projectName", node.name);
			if (node.name.empty()) {
				node.name = QFE::ConvertString(
					std::filesystem::path(QFE::ConvertString(directoryPath)).filename().wstring());
			}
			if (node.projectName.empty()) {
				node.projectName = node.name;
			}
			node.directoryPath = QFE::ConvertString(directoryPath);
			const int kind = nodeJson.value("kind", 0);
			if (kind >= 0 && kind < IM_ARRAYSIZE(kPremakeProjectKinds)) {
				node.kind = static_cast<PremakeProjectKind>(kind);
			}
			node.includePaths = nodeJson.value("includePaths", "");
			node.preBuildEvent = nodeJson.value("preBuildEvent", "");
			node.postBuildEvent = nodeJson.value("postBuildEvent", "");
			if (nodeJson.contains("position") && nodeJson["position"].is_object()) {
				const nlohmann::json& position = nodeJson["position"];
				node.initialPosition.x = position.value("x", 0.0f);
				node.initialPosition.y = position.value("y", 0.0f);
			}
			// Node Editor側のノードは次のBeginNodeで生成されるため、そこで
			// 保存した座標を適用する。
			node.positionInitialized = false;
			nodes_.push_back(std::move(node));

			if (nodeId < std::numeric_limits<std::uint64_t>::max() &&
				nextNodeId_ <= nodeId) {
				nextNodeId_ = nodeId + 1;
			}
		} catch (const nlohmann::json::exception&) {
			++invalidNodeCount;
		}
	}

	std::unordered_set<std::uint64_t> knownNodeIds;
	for (const ProjectNode& node : nodes_) {
		knownNodeIds.insert(node.id);
	}
	std::size_t invalidLinkCount = 0;
	if (configuration.contains("links") && configuration["links"].is_array()) {
		for (const nlohmann::json& linkJson : configuration["links"]) {
			try {
				ProjectLink link;
				link.id = linkJson.value("id", std::uint64_t(0));
				link.sourceNodeId = linkJson.value("sourceNodeId", std::uint64_t(0));
				link.targetNodeId = linkJson.value("targetNodeId", std::uint64_t(0));
				if (link.id == 0 || !knownNodeIds.contains(link.sourceNodeId) ||
					!knownNodeIds.contains(link.targetNodeId) ||
					LinkExists(link.sourceNodeId, link.targetNodeId)) {
					++invalidLinkCount;
					continue;
				}
				links_.push_back(link);
				if (link.id < std::numeric_limits<std::uint64_t>::max() &&
					nextLinkId_ <= link.id) {
					nextLinkId_ = link.id + 1;
				}
			} catch (const nlohmann::json::exception&) {
				++invalidLinkCount;
			}
		}
	}

	if (nodeEditorContext_ != nullptr) {
		ax::NodeEditor::SetCurrentEditor(nodeEditorContext_);
		ax::NodeEditor::ClearSelection();
		for (const ProjectNode& node : nodes_) {
			ax::NodeEditor::SetNodePosition(
				ToEditorNodeId(node.id), node.initialPosition);
		}
		ax::NodeEditor::SetCurrentEditor(nullptr);
	}

	const std::string configurationPathString =
		QFE::ConvertString(configurationPath.wstring());
	std::ostringstream status;
	status << "Loaded configuration: " << configurationPathString << " ("
		<< nodes_.size() << " node(s), " << links_.size() << " link(s)).";
	if (invalidNodeCount != 0 || invalidLinkCount != 0) {
		status << " Skipped " << invalidNodeCount << " node(s) and "
			<< invalidLinkCount << " link(s).";
	}
	status << rootLoadStatus;
	premakeStatus_ = status.str();
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
			static_cast<float>(level) * 260.0f,
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
