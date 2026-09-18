#include "ProjectGenerator.h"
#include <imgui.h>
#include "core/file/FileUtility.h"
#include "core/string/MyString.h"
#include "framework/window/WindowsWindowFrameWork.h"

#include <imgui_stdlib.h>
#include <nlohmann/json.hpp>
#include <shellapi.h>

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
		"Utility",
	};

	bool IsLibraryKind(PremakeProjectKind kind)
	{
		return kind == PremakeProjectKind::StaticLib ||
			kind == PremakeProjectKind::SharedLib;
	}

	bool CanHaveLinkInputs(PremakeProjectKind kind)
	{
		return kind != PremakeProjectKind::None &&
			kind != PremakeProjectKind::Utility;
	}

	bool IsExecutableKind(PremakeProjectKind kind)
	{
		return kind == PremakeProjectKind::ConsoleApp ||
			kind == PremakeProjectKind::WindowedApp;
	}

	bool HasCompileSourceFile(const std::filesystem::path& directoryPath)
	{
		std::error_code error;
		if (!std::filesystem::is_directory(directoryPath, error)) {
			return false;
		}

		std::filesystem::recursive_directory_iterator iterator(
			directoryPath, std::filesystem::directory_options::skip_permission_denied,
			error);
		const std::filesystem::recursive_directory_iterator end;
		while (iterator != end) {
			if (iterator->is_regular_file(error)) {
				const std::string extension =
					iterator->path().extension().generic_string();
				if (extension == ".c" || extension == ".cc" ||
					extension == ".cpp" || extension == ".cxx") {
					return true;
				}
			}
			error.clear();
			iterator.increment(error);
			if (error) {
				error.clear();
			}
		}
		return false;
	}

	PremakeProjectKind GetGeneratedProjectKind(const ProjectNode& node)
	{
		if (IsLibraryKind(node.kind) &&
			!HasCompileSourceFile(node.directoryPath)) {
			return PremakeProjectKind::Utility;
		}
		return node.kind;
	}

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
	constexpr std::uintptr_t kNodeEditorGroupIdTag =
		std::uintptr_t(5) << 60;

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

	std::uint64_t DecodeNodeEditorGroupId(ed::NodeId id)
	{
		const std::uintptr_t encoded = static_cast<std::uintptr_t>(id);
		if ((encoded & ~kNodeEditorIdPayloadMask) != kNodeEditorGroupIdTag) {
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

	ed::NodeId ToEditorGroupId(std::uint64_t id)
	{
		return ed::NodeId(EncodeNodeEditorId(kNodeEditorGroupIdTag, id));
	}

	struct ParsedPremakeProject {
		std::string name;
		std::filesystem::path directoryPath;
		PremakeProjectKind kind = PremakeProjectKind::StaticLib;
		std::string groupName;
		std::vector<std::string> includePaths;
		std::vector<std::string> preBuildCommands;
		std::vector<std::string> postBuildCommands;
		struct FilteredValue {
			std::string filter;
			std::string value;
		};
		std::vector<FilteredValue> links;
		std::vector<FilteredValue> libraryDirectories;
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
		const std::filesystem::path& scriptPath, const std::string& source,
		const std::string& inheritedGroupName)
	{
		std::vector<std::string> lines;
		std::istringstream sourceStream(source);
		for (std::string line; std::getline(sourceStream, line);) {
			lines.push_back(std::move(line));
		}

		std::vector<ParsedPremakeProject> projects;
		ParsedPremakeProject currentProject;
		std::string currentGroupName = inheritedGroupName;
		std::string currentFilter;
		bool hasProject = false;
		for (std::size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
			const std::string& line = lines[lineIndex];
			if (IsLuaCall(line, "group")) {
				TryExtractQuotedString(line, std::string_view("group").size(),
					currentGroupName);
				continue;
			}
			if (IsLuaCall(line, "filter")) {
				TryExtractQuotedString(line, std::string_view("filter").size(),
					currentFilter);
				continue;
			}
			if (IsLuaCall(line, "project")) {
				if (hasProject) {
					projects.push_back(std::move(currentProject));
				}
				currentProject = {};
				currentProject.directoryPath = scriptPath.parent_path();
				currentProject.groupName = currentGroupName;
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
			const bool isLibraryDirectoryList = IsLuaCall(line, "libdirs");
			if (!isIncludeList && !isBuildEventList && !isLinkList &&
				!isLibraryDirectoryList) {
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
			} else if (isLinkList) {
				for (const std::string& link : values) {
					currentProject.links.push_back({ currentFilter, link });
				}
			} else {
				for (const std::string& directory : values) {
					currentProject.libraryDirectories.push_back(
						{ currentFilter, directory });
				}
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
		std::size_t& missingIncludeCount,
		const std::string& inheritedGroupName)
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
			ParsePremakeProjects(normalizedPath, source, inheritedGroupName);
		projects.insert(projects.end(),
			std::make_move_iterator(parsedProjects.begin()),
			std::make_move_iterator(parsedProjects.end()));

		std::string currentGroupName = inheritedGroupName;
		std::istringstream sourceStream(source);
		for (std::string line; std::getline(sourceStream, line);) {
			if (IsLuaCall(line, "group")) {
				TryExtractQuotedString(line, std::string_view("group").size(),
					currentGroupName);
				continue;
			}
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
				missingIncludeCount, currentGroupName);
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
	// 選択中グループの設定ウィンドウ
	GroupSettingsWindow();
	// ルートPremakeの共通設定ウィンドウ
	CommonPremakeSettingsWindow();
}

void QFE::APPLICATION::ProjectGenerator::MainMenuBar()
{
	ImGui::BeginMenuBar();

	    if(ImGui::BeginMenu("File"))
    {
        if(ImGui::MenuItem("Open Root Directory"))
        {
			SelectRootDirectory();
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
		if (ImGui::MenuItem("Save Configuration As...")) {
			SaveConfiguration();
		}
		if (ImGui::MenuItem("Load Configuration...")) {
			LoadConfiguration();
		}

        ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Common Premake Settings...")) {
			commonPremakeSettingsOpen_ = true;
		}
		if (ImGui::MenuItem("Global Defines...")) {
			commonPremakeSettingsOpen_ = true;
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
	if (ImGui::Button("Change Root Directory")) {
		SelectRootDirectory();
	}

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

void QFE::APPLICATION::ProjectGenerator::SelectRootDirectory()
{
	std::wstring selectedDirectory;
	if (!QFE::FRAMEWORK::RequestGetDirectoryPathFromUser(
		imguiContext_.hwnd, L"Select a project root directory", L"*.*",
		selectedDirectory)) {
		return;
	}

	const std::filesystem::path selectedRoot =
		NormalizePath(QFE::ConvertString(selectedDirectory));
	std::error_code error;
	if (!std::filesystem::is_directory(selectedRoot, error)) {
		premakeStatus_ = "The selected path is not a directory.";
		return;
	}

	const std::string selectedDirectoryString =
		QFE::ConvertString(selectedRoot.wstring());
	if (!directoryManager_.SetLootDirectory(selectedDirectoryString)) {
		premakeStatus_ = "Could not start scanning the selected root directory.";
		return;
	}

	// ルート変更ではディレクトリ一覧だけを更新し、作成済みの構成は保持する。
	std::error_code premakeError;
	const bool hasRootPremake = std::filesystem::is_regular_file(
		selectedRoot / "premake5.lua", premakeError);
	premakeStatus_ = hasRootPremake
		? "Root directory changed. Existing nodes were kept. Use Load Root Premake to import it."
		: "Root directory changed. Existing nodes were kept; add projects or generate Premake.";
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
		if (ImGui::IsItemHovered() &&
			ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			const std::wstring directoryPath = directory.absolutePath.wstring();
			const HINSTANCE result = ShellExecuteW(
				imguiContext_.hwnd, L"open", directoryPath.c_str(), nullptr,
				nullptr, SW_SHOWNORMAL);
			if (reinterpret_cast<INT_PTR>(result) <= 32) {
				premakeStatus_ = "Could not open the selected directory in Explorer.";
			}
		}

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
	const DirectoryEntry& directory, bool placeAtVisibleCenter)
{
	if (HasNodeForDirectory(directory.absolutePath)) {
		return;
	}

	ProjectNode node;
	node.id = nextNodeId_++;
	node.name = directory.name;
	node.projectName = directory.name;
	node.directoryPath = directory.absolutePath;
	if (placeAtVisibleCenter && nodeEditorVisibleCenterInitialized_) {
		// SetNodePosition uses the node's top-left canvas coordinate.  Leave a
		// small offset so the new node itself, rather than its corner, is near
		// the center of the currently visible canvas.
		node.initialPosition = ImVec2(
			nodeEditorVisibleCenter_.x - 100.0f,
			nodeEditorVisibleCenter_.y - 50.0f);
	} else {
		node.initialPosition = ImVec2(
			static_cast<float>(directory.depth) * 260.0f,
			static_cast<float>(nodes_.size()) * 180.0f);
	}
	nodes_.emplace_back(std::move(node));
}

void QFE::APPLICATION::ProjectGenerator::AddAllDirectoryNodes()
{
	for (const DirectoryEntry& directory : directoryManager_.GetDirectories()) {
		AddNodeForDirectory(directory, false);
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
	for (ProjectGroup& group : groups_) {
		group.nodeIds.erase(
			std::remove(group.nodeIds.begin(), group.nodeIds.end(), nodeId),
			group.nodeIds.end());
	}
	groups_.erase(
		std::remove_if(
			groups_.begin(), groups_.end(),
			[this](const ProjectGroup& group) {
				if (!group.nodeIds.empty()) {
					return false;
				}
				if (selectedGroupId_ == group.id) {
					selectedGroupId_ = 0;
					groupSettingsOpen_ = false;
				}
				return true;
			}),
		groups_.end());
}

ProjectGroup* QFE::APPLICATION::ProjectGenerator::FindGroup(
	std::uint64_t groupId)
{
	const auto group = std::find_if(
		groups_.begin(), groups_.end(),
		[groupId](const ProjectGroup& value) { return value.id == groupId; });
	return group == groups_.end() ? nullptr : &(*group);
}

void QFE::APPLICATION::ProjectGenerator::RemoveGroup(std::uint64_t groupId)
{
	groups_.erase(
		std::remove_if(
			groups_.begin(), groups_.end(),
			[groupId](const ProjectGroup& group) { return group.id == groupId; }),
		groups_.end());
	if (selectedGroupId_ == groupId) {
		selectedGroupId_ = 0;
		groupSettingsOpen_ = false;
	}
}

void QFE::APPLICATION::ProjectGenerator::UpdateGroupBounds(ProjectGroup& group)
{
	if (group.nodeIds.empty()) {
		return;
	}

	ImVec2 minimum(FLT_MAX, FLT_MAX);
	ImVec2 maximum(-FLT_MAX, -FLT_MAX);
	const bool editorIsCurrent = nodeEditorContext_ != nullptr &&
		ax::NodeEditor::GetCurrentEditor() == nodeEditorContext_;
	for (const std::uint64_t nodeId : group.nodeIds) {
		const ProjectNode* node = FindNode(nodeId);
		if (node == nullptr) {
			continue;
		}

		ImVec2 position = node->initialPosition;
		ImVec2 nodeSize(180.0f, 100.0f);
		if (editorIsCurrent) {
			const ImVec2 editorPosition =
				ax::NodeEditor::GetNodePosition(ToEditorNodeId(nodeId));
			const ImVec2 editorSize =
				ax::NodeEditor::GetNodeSize(ToEditorNodeId(nodeId));
			if (editorPosition.x != FLT_MAX && editorPosition.y != FLT_MAX) {
				position = editorPosition;
			}
			if (editorSize.x > 0.0f && editorSize.y > 0.0f) {
				nodeSize = editorSize;
			}
		}

		minimum.x = std::min(minimum.x, position.x);
		minimum.y = std::min(minimum.y, position.y);
		maximum.x = std::max(maximum.x, position.x + nodeSize.x);
		maximum.y = std::max(maximum.y, position.y + nodeSize.y);
	}

	if (minimum.x == FLT_MAX || minimum.y == FLT_MAX) {
		return;
	}

	const ImVec2 padding(48.0f, 48.0f);
	group.initialPosition = ImVec2(
		minimum.x - padding.x, minimum.y - padding.y);
	group.size = ImVec2(
		maximum.x - minimum.x + padding.x * 2.0f,
		maximum.y - minimum.y + padding.y * 2.0f);
	group.size.x = std::max(group.size.x, 240.0f);
	group.size.y = std::max(group.size.y, 160.0f);
	group.positionInitialized = false;
}

void QFE::APPLICATION::ProjectGenerator::DrawProjectGroup(ProjectGroup& group)
{
	const ax::NodeEditor::NodeId editorGroupId = ToEditorGroupId(group.id);
	if (!group.positionInitialized) {
		ax::NodeEditor::SetNodePosition(editorGroupId, group.initialPosition);
		ax::NodeEditor::SetGroupSize(editorGroupId, group.size);
		group.positionInitialized = true;
	}

	ax::NodeEditor::BeginNode(editorGroupId);
	ImGui::PushID(reinterpret_cast<void*>(
		static_cast<std::uintptr_t>(group.id)));
	const ImVec2 labelPosition = ImGui::GetCursorScreenPos();
	ImGui::GetWindowDrawList()->AddText(
		labelPosition, IM_COL32(235, 235, 235, 255),
		group.name.empty() ? "Group" : group.name.c_str());
	ax::NodeEditor::Group(group.size);
	ax::NodeEditor::EndNode();
	ImGui::PopID();
}

void QFE::APPLICATION::ProjectGenerator::GroupSelectedNodes(
	const std::vector<std::uint64_t>& selectedNodeIds)
{
	std::vector<std::uint64_t> validNodeIds;
	for (const std::uint64_t nodeId : selectedNodeIds) {
		if (FindNode(nodeId) != nullptr &&
			std::find(validNodeIds.begin(), validNodeIds.end(), nodeId) ==
			validNodeIds.end()) {
			validNodeIds.push_back(nodeId);
		}
	}
	if (validNodeIds.size() < 2) {
		return;
	}

	ProjectGroup group;
	group.id = nextGroupId_++;
	group.name = "Group_" + std::to_string(group.id);
	group.nodeIds = validNodeIds;
	UpdateGroupBounds(group);
	group.needsBoundsUpdate = false;

	for (ProjectGroup& existingGroup : groups_) {
		existingGroup.nodeIds.erase(
			std::remove_if(
				existingGroup.nodeIds.begin(), existingGroup.nodeIds.end(),
				[&validNodeIds](std::uint64_t nodeId) {
					return std::find(validNodeIds.begin(), validNodeIds.end(), nodeId) !=
						validNodeIds.end();
				}),
			existingGroup.nodeIds.end());
	}
	groups_.erase(
		std::remove_if(
			groups_.begin(), groups_.end(),
			[](const ProjectGroup& existingGroup) {
				return existingGroup.nodeIds.empty();
			}),
		groups_.end());

	groups_.push_back(std::move(group));
	selectedGroupId_ = groups_.back().id;
	groupSettingsOpen_ = true;
	selectedNodeId_ = 0;
	nodeSettingsOpen_ = false;

	if (nodeEditorContext_ != nullptr &&
		ax::NodeEditor::GetCurrentEditor() == nodeEditorContext_) {
		const ProjectGroup& createdGroup = groups_.back();
		const ax::NodeEditor::NodeId editorGroupId =
			ToEditorGroupId(createdGroup.id);
		ax::NodeEditor::SetNodePosition(
			editorGroupId, createdGroup.initialPosition);
		ax::NodeEditor::SetGroupSize(editorGroupId, createdGroup.size);
		ax::NodeEditor::ClearSelection();
		ax::NodeEditor::SelectNode(editorGroupId);
	}
	premakeStatus_ = "Grouped " + std::to_string(validNodeIds.size()) +
		" node(s). Set the Premake group name in Group Settings.";
}

void QFE::APPLICATION::ProjectGenerator::DeleteSelectedNodes(
	const std::vector<std::uint64_t>& selectedNodeIds)
{
	std::vector<std::uint64_t> validNodeIds;
	for (const std::uint64_t nodeId : selectedNodeIds) {
		if (FindNode(nodeId) != nullptr &&
			std::find(validNodeIds.begin(), validNodeIds.end(), nodeId) ==
			validNodeIds.end()) {
			validNodeIds.push_back(nodeId);
		}
	}
	for (const std::uint64_t nodeId : validNodeIds) {
		RemoveNode(nodeId);
	}
	if (nodeEditorContext_ != nullptr &&
		ax::NodeEditor::GetCurrentEditor() == nodeEditorContext_) {
		ax::NodeEditor::ClearSelection();
	}
	selectedNodeId_ = 0;
	selectedGroupId_ = 0;
	nodeSettingsOpen_ = false;
	groupSettingsOpen_ = false;
	premakeStatus_ = "Deleted " + std::to_string(validNodeIds.size()) +
		" selected node(s).";
}

void QFE::APPLICATION::ProjectGenerator::DrawNodeContextMenu(
	const std::vector<std::uint64_t>& selectedNodeIds)
{
	if (selectedNodeIds.empty()) {
		return;
	}
	if (!ImGui::BeginPopupContextWindow(
		"NodeEditorContextMenu", ImGuiPopupFlags_MouseButtonRight)) {
		return;
	}

	ImGui::Text("Selected nodes: %d",
		static_cast<int>(selectedNodeIds.size()));
	ImGui::Separator();
	if (ImGui::MenuItem(
		"Group Selected Nodes", nullptr, false, selectedNodeIds.size() >= 2)) {
		GroupSelectedNodes(selectedNodeIds);
	}
	if (ImGui::MenuItem("Delete Selected Nodes")) {
		DeleteSelectedNodes(selectedNodeIds);
	}
	ImGui::EndPopup();
}

void QFE::APPLICATION::ProjectGenerator::GroupSettingsWindow()
{
	if (!groupSettingsOpen_ || selectedGroupId_ == 0) {
		return;
	}

	ProjectGroup* group = FindGroup(selectedGroupId_);
	if (group == nullptr) {
		selectedGroupId_ = 0;
		groupSettingsOpen_ = false;
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(360.0f, 220.0f), ImGuiCond_FirstUseEver);
	bool isOpen = groupSettingsOpen_;
	bool shouldRemove = false;
	if (ImGui::Begin("Group Settings", &isOpen)) {
		ImGui::Text("Premake group");
		ImGui::InputText("Name", &group->name);
		ImGui::Text("Members: %d", static_cast<int>(group->nodeIds.size()));
		ImGui::TextWrapped(
			"The name is emitted as Premake: group \"name\".");
		if (ImGui::Button("Ungroup")) {
			shouldRemove = true;
		}
	}
	ImGui::End();

	if (shouldRemove) {
		RemoveGroup(group->id);
	} else if (!isOpen) {
		selectedGroupId_ = 0;
		groupSettingsOpen_ = false;
	}
}

void QFE::APPLICATION::ProjectGenerator::CommonPremakeSettingsWindow()
{
	if (!commonPremakeSettingsOpen_) {
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(520.0f, 680.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(
		ImVec2(380.0f, 420.0f), ImVec2(FLT_MAX, FLT_MAX));
	bool isOpen = commonPremakeSettingsOpen_;
	if (ImGui::Begin("Common Premake Settings", &isOpen)) {
		ImGui::TextWrapped(
			"These settings are written to the root premake5.lua and inherited by every project.");
		ImGui::Separator();

		ImGui::TextUnformatted("Workspace name");
		ImGui::InputText("##CommonWorkspaceName",
			&commonPremakeSettings_.workspaceName);

		ImGui::TextUnformatted("Architecture");
		ImGui::InputText("##CommonArchitecture",
			&commonPremakeSettings_.architecture);

		ImGui::TextUnformatted("C++ dialect");
		ImGui::InputText("##CommonCppDialect",
			&commonPremakeSettings_.cppDialect);

		ImGui::Checkbox("Static runtime",
			&commonPremakeSettings_.staticRuntime);

		ImGui::Separator();
		ImGui::TextUnformatted("Debug working directory");
		ImGui::PushItemWidth(-110.0f);
		ImGui::InputText("##CommonDebugDirectory",
			&commonPremakeSettings_.debugDirectory);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("Browse##CommonDebugDirectory")) {
			std::wstring selectedDirectory;
			if (QFE::FRAMEWORK::RequestGetDirectoryPathFromUser(
				imguiContext_.hwnd, L"Select debug working directory", L"*.*",
				selectedDirectory)) {
				const std::filesystem::path selectedPath = NormalizePath(
					QFE::ConvertString(selectedDirectory));
				const std::filesystem::path rootPath = NormalizePath(
					QFE::ConvertString(directoryManager_.GetLootDirectory()));
				std::error_code relativeError;
				const std::filesystem::path relativePath =
					std::filesystem::relative(selectedPath, rootPath, relativeError);
				if (!relativeError && !relativePath.empty()) {
					commonPremakeSettings_.debugDirectory =
						relativePath.generic_string();
				} else {
					commonPremakeSettings_.debugDirectory =
						selectedPath.generic_string();
				}
			}
		}
		ImGui::TextWrapped(
			"Path relative to the root directory. Premake tokens such as "
			"%%{cfg.buildcfg} and %%{cfg.platform} are supported.");

		ImGui::TextUnformatted("Configurations (one per line)");
		ImGui::InputTextMultiline(
			"##CommonConfigurations", &commonPremakeSettings_.configurations,
			ImVec2(-FLT_MIN, 72.0f), ImGuiInputTextFlags_AllowTabInput);

		ImGui::TextUnformatted("Flags (one per line)");
		ImGui::InputTextMultiline(
			"##CommonFlags", &commonPremakeSettings_.flags,
			ImVec2(-FLT_MIN, 64.0f), ImGuiInputTextFlags_AllowTabInput);

		ImGui::TextUnformatted("Build options (one option per line)");
		ImGui::InputTextMultiline(
			"##CommonBuildOptions", &commonPremakeSettings_.buildOptions,
			ImVec2(-FLT_MIN, 72.0f), ImGuiInputTextFlags_AllowTabInput);

		ImGui::TextUnformatted("Common include paths (one path per line)");
		ImGui::InputTextMultiline(
			"##CommonIncludePaths", &commonPremakeSettings_.includePaths,
			ImVec2(-FLT_MIN, 88.0f), ImGuiInputTextFlags_AllowTabInput);

		ImGui::Separator();
		ImGui::TextUnformatted("Global defines (one definition per line)");
		ImGui::TextWrapped(
			"These definitions are written to the root Premake workspace and every generated project. Examples: QFE_FEATURE_X or QFE_VERSION=1.");
		ImGui::InputTextMultiline(
			"##CommonDefines", &commonPremakeSettings_.defines,
			ImVec2(-FLT_MIN, 72.0f), ImGuiInputTextFlags_AllowTabInput);

		ImGui::Separator();
		if (ImGui::Button("Generate Central Premake")) {
			GenerateCentralPremake();
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset Defaults")) {
			commonPremakeSettings_ = PremakeCommonSettings{};
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			isOpen = false;
		}
	}
	ImGui::End();

	commonPremakeSettingsOpen_ = isOpen;
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
	const ImVec2 canvasScreenTopLeft = ImGui::GetCursorScreenPos();

	ax::NodeEditor::SetCurrentEditor(nodeEditorContext_);
	ax::NodeEditor::Begin("Project Dependency Graph", canvasSize);
	nodeEditorVisibleCenter_ = ax::NodeEditor::ScreenToCanvas(ImVec2(
		canvasScreenTopLeft.x + canvasSize.x * 0.5f,
		canvasScreenTopLeft.y + canvasSize.y * 0.5f));
	nodeEditorVisibleCenterInitialized_ = true;

	for (ProjectGroup& group : groups_) {
		DrawProjectGroup(group);
	}

	for (ProjectNode& node : nodes_) {
		const ax::NodeEditor::NodeId editorNodeId = ToEditorNodeId(node.id);
		if (!node.positionInitialized) {
			ax::NodeEditor::SetNodePosition(editorNodeId, node.initialPosition);
			node.positionInitialized = true;
		}

		DrawProjectNode(node);
	}
	for (ProjectGroup& group : groups_) {
		if (!group.needsBoundsUpdate) {
			continue;
		}
		UpdateGroupBounds(group);
		group.needsBoundsUpdate = false;
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

	// Node Editorの選択状態をモデルIDへ変換する。グループと通常ノードは
	// ID領域が異なるため、右クリックメニューと設定ウィンドウで安全に
	// 別扱いできる。
	const int selectedObjectCount = ax::NodeEditor::GetSelectedNodes(nullptr, 0);
	std::vector<ax::NodeEditor::NodeId> selectedEditorNodes(
		static_cast<std::size_t>(std::max(selectedObjectCount, 0)));
	if (!selectedEditorNodes.empty()) {
		ax::NodeEditor::GetSelectedNodes(
			selectedEditorNodes.data(), selectedObjectCount);
	}
	std::vector<std::uint64_t> selectedNodeIds;
	std::vector<std::uint64_t> selectedGroupIds;
	for (const ax::NodeEditor::NodeId selectedEditorNode : selectedEditorNodes) {
		const std::uint64_t nodeId =
			DecodeNodeEditorNodeId(selectedEditorNode);
		if (nodeId != 0 && FindNode(nodeId) != nullptr) {
			selectedNodeIds.push_back(nodeId);
			continue;
		}
		const std::uint64_t groupId =
			DecodeNodeEditorGroupId(selectedEditorNode);
		if (groupId != 0 && FindGroup(groupId) != nullptr) {
			selectedGroupIds.push_back(groupId);
		}
	}

	if (!selectedGroupIds.empty()) {
		selectedGroupId_ = selectedGroupIds.front();
		groupSettingsOpen_ = true;
		selectedNodeId_ = 0;
		nodeSettingsOpen_ = false;
	} else if (!selectedNodeIds.empty()) {
		selectedGroupId_ = 0;
		groupSettingsOpen_ = false;
		if (selectedNodeId_ != selectedNodeIds.front()) {
			selectedNodeId_ = selectedNodeIds.front();
		}
		nodeSettingsOpen_ = true;
	} else {
		selectedNodeId_ = 0;
		nodeSettingsOpen_ = false;
		selectedGroupId_ = 0;
		groupSettingsOpen_ = false;
	}

	DrawNodeContextMenu(selectedNodeIds);

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
					SimplifyDependencyLinks();
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

void QFE::APPLICATION::ProjectGenerator::CollectDependencyClosure(
	std::uint64_t targetNodeId,
	std::vector<std::uint64_t>& dependencyNodeIds,
	std::unordered_set<std::uint64_t>& visitedNodeIds) const
{
	for (const ProjectLink& link : links_) {
		if (link.targetNodeId != targetNodeId ||
			link.sourceNodeId == targetNodeId) {
			continue;
		}
		if (!visitedNodeIds.insert(link.sourceNodeId).second) {
			continue;
		}

		dependencyNodeIds.push_back(link.sourceNodeId);
		CollectDependencyClosure(link.sourceNodeId, dependencyNodeIds,
			visitedNodeIds);
	}
}

void QFE::APPLICATION::ProjectGenerator::SimplifyDependencyLinks()
{
	std::unordered_set<std::uint64_t> redundantLinkIds;
	for (const ProjectLink& candidate : links_) {
		std::vector<std::uint64_t> pendingNodes{ candidate.sourceNodeId };
		std::unordered_set<std::uint64_t> visitedNodeIds;
		bool hasAlternatePath = false;

		while (!pendingNodes.empty() && !hasAlternatePath) {
			const std::uint64_t currentNodeId = pendingNodes.back();
			pendingNodes.pop_back();
			if (!visitedNodeIds.insert(currentNodeId).second) {
				continue;
			}

			for (const ProjectLink& link : links_) {
				if (link.id == candidate.id ||
					link.sourceNodeId != currentNodeId) {
					continue;
				}
				if (link.targetNodeId == candidate.targetNodeId) {
					hasAlternatePath = true;
					break;
				}
				pendingNodes.push_back(link.targetNodeId);
			}
		}

		if (hasAlternatePath) {
			redundantLinkIds.insert(candidate.id);
		}
	}

	links_.erase(
		std::remove_if(
			links_.begin(), links_.end(),
			[&redundantLinkIds](const ProjectLink& link) {
				return redundantLinkIds.contains(link.id);
			}),
		links_.end());
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
		missingIncludeCount, {});
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
				AddNodeForDirectory(directory, false);
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
		node->externalLinks.clear();
		node->libraryDirectories.clear();
		++matchedProjectCount;
	}

	// Premakeのgroup宣言をグループモデルへ反映する。読み込み時は
	// Premakeを正とし、存在しないgroupやメンバーは構成から外す。
	groups_.clear();
	nextGroupId_ = 1;
	for (const ParsedPremakeProject& project : projects) {
		const std::string groupName = Trim(project.groupName);
		if (groupName.empty()) {
			continue;
		}
		const std::string pathKey =
			NormalizePath(project.directoryPath).generic_string();
		const auto nodeId = nodeIdsByPath.find(pathKey);
		if (nodeId == nodeIdsByPath.end()) {
			continue;
		}

		ProjectGroup* group = nullptr;
		for (ProjectGroup& candidate : groups_) {
			if (candidate.name == groupName) {
				group = &candidate;
				break;
			}
		}
		if (group == nullptr) {
			groups_.push_back({});
			group = &groups_.back();
			group->id = nextGroupId_++;
			group->name = groupName;
			group->needsBoundsUpdate = true;
		}
		if (std::find(group->nodeIds.begin(), group->nodeIds.end(), nodeId->second) ==
			group->nodeIds.end()) {
			group->nodeIds.push_back(nodeId->second);
		}
	}
	for (ProjectGroup& group : groups_) {
		UpdateGroupBounds(group);
	}
	selectedGroupId_ = 0;
	groupSettingsOpen_ = false;

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
	for (const ParsedPremakeProject& project : projects) {
		const auto target = nodeIdsByProjectName.find(project.name);
		if (target == nodeIdsByProjectName.end()) {
			continue;
		}
		ProjectNode* targetNode = FindNode(target->second);
		if (targetNode == nullptr) {
			continue;
		}
		for (const ParsedPremakeProject::FilteredValue& dependency : project.links) {
			const auto source = nodeIdsByProjectName.find(dependency.value);
			if (source == nodeIdsByProjectName.end()) {
				if (!dependency.value.empty()) {
					targetNode->externalLinks.push_back({
						dependency.filter, dependency.value });
				}
				continue;
			}
			if (source->second == target->second ||
				LinkExists(source->second, target->second) ||
				WouldCreateCycle(source->second, target->second)) {
				continue;
			}
			links_.push_back({ nextLinkId_++, source->second, target->second });
		}
		for (const ParsedPremakeProject::FilteredValue& directory :
			project.libraryDirectories) {
			if (!directory.value.empty()) {
				targetNode->libraryDirectories.push_back({
					directory.filter, directory.value });
			}
		}
	}
	const std::size_t importedLinkCountBeforeSimplification = links_.size();
	SimplifyDependencyLinks();
	const std::size_t simplifiedLinkCount =
		importedLinkCountBeforeSimplification - links_.size();

	std::ostringstream status;
	status << "Loaded " << visitedFiles.size() << " Premake file(s), matched "
		<< matchedProjectCount << "/" << projects.size() << " project(s), imported "
		<< links_.size() << " direct link(s), " << groups_.size() << " group(s).";
	if (simplifiedLinkCount != 0) {
		status << " Omitted " << simplifiedLinkCount
			<< " redundant transitive link(s).";
	}
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
	const std::filesystem::path buildPath = rootPath / "build";
	const std::filesystem::path projectsPath = buildPath / "projects";
	std::error_code directoryError;
	std::filesystem::create_directories(projectsPath, directoryError);
	if (directoryError) {
		premakeStatus_ = "Could not create the build/projects directory.";
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

	std::unordered_map<std::uint64_t, std::string> generatedProjectDirectories;
	std::unordered_set<std::string> usedProjectDirectories;
	for (const ProjectNode& node : nodes_) {
		std::string directoryName = generatedNames[node.id];
		for (char& character : directoryName) {
			const bool isForbidden =
				static_cast<unsigned char>(character) < 0x20 ||
				std::string_view("<>:\"/\\|?*").find(character) !=
				std::string_view::npos;
			if (isForbidden) {
				character = '_';
			}
		}
		while (!directoryName.empty() &&
			(directoryName.back() == '.' || directoryName.back() == ' ')) {
			directoryName.pop_back();
		}
		if (directoryName.empty() || directoryName == "." || directoryName == "..") {
			directoryName = "Project";
		}
		if (!usedProjectDirectories.insert(directoryName).second) {
			directoryName += "_" + std::to_string(node.id);
			usedProjectDirectories.insert(directoryName);
		}
		generatedProjectDirectories[node.id] = std::move(directoryName);
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

	auto writeStringList = [](std::ostream& output, const char* setting,
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
	auto writeLuaExpressionList = [](std::ostream& output, const char* setting,
		const std::vector<std::string>& expressions) {
		if (expressions.empty()) {
			return;
		}
		output << "    " << setting << " {\n";
		for (const std::string& expression : expressions) {
			if (!Trim(expression).empty()) {
				output << "        " << expression << ",\n";
			}
		}
		output << "    }\n";
	};

	const std::string buildRootString = buildPath.generic_string();
	auto resolveIncludePath = [&rootPath, &buildRootString](const std::string& value) {
		std::string resolved = Trim(value);
		const std::string workspaceToken = "%{wks.location}";
		std::size_t tokenPosition = resolved.find(workspaceToken);
		while (tokenPosition != std::string::npos) {
			// 既存の構成ファイルには、元のworkspaceがsource rootを指して
			// いた時の "%{wks.location}/engine" と、workspaceがbuildを
			// 指していた時の "%{wks.location}/../engine" が混在している。
			// 実在する候補を優先して、生成後のパスを壊さないようにする。
			std::string sourceCandidate = resolved;
			sourceCandidate.replace(
				tokenPosition, workspaceToken.size(), rootPath.generic_string());
			std::string buildCandidate = resolved;
			buildCandidate.replace(
				tokenPosition, workspaceToken.size(), buildRootString);
			std::error_code sourceError;
			std::error_code buildError;
			const bool sourceExists = std::filesystem::exists(
				NormalizePath(QFE::ConvertString(sourceCandidate)), sourceError);
			const bool buildExists = std::filesystem::exists(
				NormalizePath(QFE::ConvertString(buildCandidate)), buildError);
			const std::string& replacement = sourceExists && !buildExists
				? rootPath.generic_string()
				: buildRootString;
			resolved.replace(tokenPosition, workspaceToken.size(), replacement);
			tokenPosition = resolved.find(workspaceToken,
				tokenPosition + replacement.size());
		}

		// 生成スクリプトはbuildをworkspace locationにするため、ルート相対の
		// include pathを共通ルート変数から解決する。
		if (resolved.find("%{") == std::string::npos &&
			resolved.find("$(") == std::string::npos) {
			std::filesystem::path includePath = QFE::ConvertString(resolved);
			if (!includePath.is_absolute()) {
				includePath = rootPath / includePath;
			}

			const std::filesystem::path normalizedIncludePath =
				NormalizePath(includePath);
			std::error_code relativeError;
			const std::filesystem::path relativePath =
				std::filesystem::relative(normalizedIncludePath, rootPath, relativeError);
			const std::string relativePathString = relativePath.generic_string();
			const bool isRootRelative = !relativeError &&
				!relativePathString.empty() && relativePathString != ".." &&
				relativePathString.rfind("../", 0) != 0;
			if (isRootRelative) {
				return "path.join(QFE_PROJECT_ROOT, \"" +
					EscapeLuaString(relativePathString) + "\")";
			}
			resolved = normalizedIncludePath.generic_string();
		} else {
			// Premakeの変数式は文字列リテラルにせず、そのまま評価させる。
			return resolved;
		}
		return "\"" + EscapeLuaString(resolved) + "\"";
	};
	// Convert the editor's global debug directory setting into a Premake
	// expression.  A path containing configuration tokens must stay inside
	// path.join so %{cfg.*} is evaluated by Premake later.
	auto resolveDebugDirectory = [&resolveIncludePath](const std::string& value) {
		const std::string resolved = Trim(value);
		if (resolved.empty()) {
			return std::string{};
		}
		if (resolved.find("%{") != std::string::npos ||
			resolved.find("$(") != std::string::npos) {
			return "path.join(QFE_PROJECT_ROOT, \"" +
				EscapeLuaString(resolved) + "\")";
		}
		return resolveIncludePath(resolved);
	};

	std::vector<std::string> commonConfigurations =
		SplitLines(commonPremakeSettings_.configurations);
	if (commonConfigurations.empty()) {
		commonConfigurations = { "Debug", "Development", "Release" };
	}
	const std::string commonWorkspaceName =
		Trim(commonPremakeSettings_.workspaceName).empty()
		? "QuickForgeEngine"
		: Trim(commonPremakeSettings_.workspaceName);
	const std::string commonArchitecture =
		Trim(commonPremakeSettings_.architecture).empty()
		? "x64"
		: Trim(commonPremakeSettings_.architecture);
	const std::string commonCppDialect =
		Trim(commonPremakeSettings_.cppDialect).empty()
		? "C++20"
		: Trim(commonPremakeSettings_.cppDialect);
	// These are the include roots shared by the engine projects.  They are
	// emitted at project scope because Premake workspace-level include dirs do
	// not reliably propagate to every generated Visual Studio project.
	std::vector<std::string> commonIncludePathExpressions = {
		"path.join(QFE_PROJECT_ROOT, \"engine\")",
		"path.join(QFE_PROJECT_ROOT, \"engine/core\")",
		"path.join(QFE_PROJECT_ROOT, \"externals\")",
		"path.join(QFE_PROJECT_ROOT, \"externals/imgui\")",
		"path.join(QFE_PROJECT_ROOT, \"externals/assimp\")",
		"path.join(QFE_PROJECT_ROOT, \"externals/assimp/include\")",
		"path.join(QFE_PROJECT_ROOT, \"externals/imgui/imgui-node-editor-0.9.3\")",
	};
	for (const std::string& includePath :
		SplitLines(commonPremakeSettings_.includePaths)) {
		commonIncludePathExpressions.push_back(resolveIncludePath(includePath));
	}
	const std::vector<std::string> commonFlags =
		SplitLines(commonPremakeSettings_.flags);
	const std::vector<std::string> commonBuildOptions =
		SplitLines(commonPremakeSettings_.buildOptions);
	const std::vector<std::string> commonDefines =
		SplitLines(commonPremakeSettings_.defines);
	const std::string commonDebugDirectoryExpression =
		resolveDebugDirectory(commonPremakeSettings_.debugDirectory);

	auto writeProject = [&](const ProjectNode& node) {
		const std::filesystem::path projectScriptDirectory =
			projectsPath / generatedProjectDirectories[node.id];
		std::error_code projectDirectoryError;
		std::filesystem::create_directories(
			projectScriptDirectory, projectDirectoryError);
		if (projectDirectoryError) {
			return false;
		}

		const std::filesystem::path projectScriptPath =
			projectScriptDirectory / "premake.lua";
		std::ofstream output(projectScriptPath, std::ios::trunc);
		if (!output) {
			return false;
		}

		const std::string& projectName = generatedNames[node.id];
		const std::string projectRelativeDirectory =
			relativeDirectory(node.directoryPath);
		const PremakeProjectKind generatedKind = GetGeneratedProjectKind(node);

		output << "-- Generated by QuickForgeEngine ProjectGenerator.\n";
		output << "local _sourceDirectory = path.getabsolute(path.join("
			<< "QFE_PROJECT_ROOT, \""
			<< EscapeLuaString(projectRelativeDirectory) << "\"))\n";
		output << "local _projectDirectory = path.getabsolute(path.join("
			<< "QFE_BUILD_ROOT, \"projects/"
			<< EscapeLuaString(generatedProjectDirectories[node.id])
			<< "\"))\n\n";
		output << "project \"" << EscapeLuaString(projectName) << "\"\n";
		output << "    location (_projectDirectory)\n";
		output << "    kind \"" << EscapeLuaString(GetPremakeKindName(generatedKind)) << "\"\n";
		output << "    language \"C++\"\n";
		output << "    objdir (path.join(QFE_PROJECT_ROOT, \"../generated/obj/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}\"))\n";
		output << "    targetdir (path.join(QFE_PROJECT_ROOT, \"../generated/outputs/%{cfg.buildcfg}/%{cfg.platform}\"))\n";
		if (IsExecutableKind(generatedKind) &&
			!commonDebugDirectoryExpression.empty()) {
			output << "    debugdir (" << commonDebugDirectoryExpression << ")\n";
		}
		output << "    files {\n";
		output << "        path.join(_sourceDirectory, \"**.h\"),\n";
		output << "        path.join(_sourceDirectory, \"**.cpp\"),\n";
		output << "    }\n";

		std::vector<std::string> includePathExpressions;
		for (const std::string& includePath : SplitLines(node.includePaths)) {
			includePathExpressions.push_back(resolveIncludePath(includePath));
		}
		includePathExpressions.insert(
			includePathExpressions.end(), commonIncludePathExpressions.begin(),
			commonIncludePathExpressions.end());
		writeLuaExpressionList(output, "includedirs", includePathExpressions);
		// Premakeのworkspace設定だけでは、生成した各プロジェクトへ
		// 伝播しない項目があるため、共通設定はprojectスコープにも出力する。
		writeStringList(output, "flags", commonFlags);
		writeStringList(output, "buildoptions", commonBuildOptions);
		writeStringList(output, "defines", commonDefines);
		std::vector<std::uint64_t> dependencyNodeIds;
		std::unordered_set<std::uint64_t> visitedDependencyIds;
		CollectDependencyClosure(node.id, dependencyNodeIds, visitedDependencyIds);
		std::vector<std::string> nodeLinks;
		for (const std::uint64_t dependencyNodeId : dependencyNodeIds) {
			const ProjectNode* dependencyNode = FindNode(dependencyNodeId);
			if (dependencyNode == nullptr ||
				!IsLibraryKind(GetGeneratedProjectKind(*dependencyNode))) {
				// Utility/None/実行ファイルはリンク入力にならないため、
				// Premakeのlinksには出力しない。
				continue;
			}
			const auto sourceName = generatedNames.find(dependencyNodeId);
			if (sourceName != generatedNames.end()) {
				nodeLinks.push_back(sourceName->second);
			}
		}
		if (CanHaveLinkInputs(generatedKind)) {
			writeStringList(output, "links", nodeLinks);

			// 静的ライブラリの外部リンクは、そのライブラリを使う最終
			// ターゲットにも必要になる。依存関係をたどって設定を集約する。
			std::vector<const ProjectNode*> dependencySettingsNodes{ &node };
			for (const std::uint64_t dependencyNodeId : dependencyNodeIds) {
				const ProjectNode* dependencyNode = FindNode(dependencyNodeId);
				if (dependencyNode != nullptr) {
					dependencySettingsNodes.push_back(dependencyNode);
				}
			}

			std::vector<ProjectNode::ExternalLink> externalLinks;
			std::unordered_set<std::string> externalLinkKeys;
			std::vector<ProjectNode::LibraryDirectory> libraryDirectories;
			std::unordered_set<std::string> libraryDirectoryKeys;
			for (const ProjectNode* settingsNode : dependencySettingsNodes) {
				for (const ProjectNode::ExternalLink& externalLink :
					settingsNode->externalLinks) {
					const std::string key = externalLink.filter + "\n" + externalLink.name;
					if (externalLinkKeys.insert(key).second) {
						externalLinks.push_back(externalLink);
					}
				}
				for (const ProjectNode::LibraryDirectory& libraryDirectory :
					settingsNode->libraryDirectories) {
					const std::string key = libraryDirectory.filter + "\n" +
						libraryDirectory.path;
					if (libraryDirectoryKeys.insert(key).second) {
						libraryDirectories.push_back(libraryDirectory);
					}
				}
			}

			auto writeFilteredLists =
				[&output](const auto& entries, const char* setting,
					auto getFilter, auto getValue, auto writeValues) {
					std::vector<std::string> filters;
					std::unordered_map<std::string, std::vector<std::string>> valuesByFilter;
					for (const auto& entry : entries) {
						const std::string filter = getFilter(entry);
						const std::string value = getValue(entry);
						if (Trim(value).empty()) {
							continue;
						}
						if (!valuesByFilter.contains(filter)) {
							filters.push_back(filter);
						}
						valuesByFilter[filter].push_back(value);
					}

					for (const std::string& filter : filters) {
						if (!Trim(filter).empty()) {
							output << "    filter \""
								<< EscapeLuaString(filter) << "\"\n";
						}
						writeValues(output, setting, valuesByFilter[filter]);
						if (!Trim(filter).empty()) {
							output << "    filter \"\"\n";
						}
					}
				};

			writeFilteredLists(externalLinks, "links",
				[](const ProjectNode::ExternalLink& entry) { return entry.filter; },
				[](const ProjectNode::ExternalLink& entry) { return entry.name; },
				writeStringList);
			writeFilteredLists(libraryDirectories, "libdirs",
				[](const ProjectNode::LibraryDirectory& entry) {
					return entry.filter;
				},
				[&resolveIncludePath](const ProjectNode::LibraryDirectory& entry) {
					return resolveIncludePath(entry.path);
				},
				writeLuaExpressionList);
		}

		auto resolveBuildCommand = [&node](const std::string& value) {
			const std::string command = Trim(value);
			if (command == "cd Shaders && CompileShaders.cmd") {
				return std::string("cd /d \"") +
					NormalizePath(node.directoryPath / "Shaders").generic_string() +
					"\" && CompileShaders.cmd";
			}
			return command;
		};
		std::vector<std::string> preBuildCommands;
		for (const std::string& command : SplitLines(node.preBuildEvent)) {
			preBuildCommands.push_back(resolveBuildCommand(command));
		}
		std::vector<std::string> postBuildCommands;
		for (const std::string& command : SplitLines(node.postBuildEvent)) {
			postBuildCommands.push_back(resolveBuildCommand(command));
		}
		writeStringList(output, "prebuildcommands", preBuildCommands);
		writeStringList(output, "postbuildcommands", postBuildCommands);
		output << "\n";
		output.flush();
		return static_cast<bool>(output);
	};

	for (const ProjectNode& node : nodes_) {
		if (!writeProject(node)) {
			premakeStatus_ = "Could not write a project Premake script.";
			return;
		}
	}

	const std::filesystem::path rootPremakePath = buildPath / "premake5.lua";
	std::ofstream output(rootPremakePath, std::ios::trunc);
	if (!output) {
		premakeStatus_ = "Could not write build/premake5.lua.";
		return;
	}

	output << "-- Generated by QuickForgeEngine ProjectGenerator.\n";
	output << "-- Run Premake from this build directory.\n";
	output << "local _buildRoot = path.getabsolute(path.getdirectory(_SCRIPT))\n";
	output << "-- Change only this value when the source root is moved separately.\n";
	output << "local _sourceRootOverride = \"\"\n";
	output << "local _sourceRoot = _sourceRootOverride ~= \"\" and "
		<< "path.getabsolute(_sourceRootOverride) or "
		<< "path.getabsolute(path.join(_buildRoot, \"..\"))\n";
	output << "QFE_BUILD_ROOT = _buildRoot\n";
	output << "QFE_PROJECT_ROOT = _sourceRoot\n\n";
	output << "workspace \"" << EscapeLuaString(commonWorkspaceName) << "\"\n";
	output << "    location (_buildRoot)\n";
	output << "    architecture \"" << EscapeLuaString(commonArchitecture) << "\"\n";
	writeStringList(output, "configurations", commonConfigurations);
	output << "    cppdialect \"" << EscapeLuaString(commonCppDialect) << "\"\n";
	output << "    staticruntime \""
		<< (commonPremakeSettings_.staticRuntime ? "on" : "off") << "\"\n";
	// WindowEventsManager forwards Windows input messages to ImGui when
	// USE_IMGUI is enabled.  Keep the configuration defines from the original
	// workspace in generated Premake so generated projects remain interactive.
	output << "    filter \"configurations:Debug\"\n";
	output << "        defines { \"_DEBUG\", \"QFE_OPTIMIZE_OFF\", "
		<< "\"QFE_MODE_DEBUG\", \"USE_IMGUI\" }\n";
	output << "    filter \"configurations:Development\"\n";
	output << "        defines { \"NDEBUG\", \"QFE_OPTIMIZE_OFF\", "
		<< "\"QFE_MODE_DEVELOPMENT\", \"USE_IMGUI\" }\n";
	output << "    filter \"configurations:Release\"\n";
	output << "        defines { \"NDEBUG\", \"QFE_OPTIMIZE_ON\", "
		<< "\"QFE_MODE_RELEASE\", \"NO_IMGUI\" }\n";
	output << "    filter \"configurations:Debug\"\n";
	output << "        runtime \"Debug\"\n";
	output << "        optimize \"Off\"\n";
	output << "        symbols \"On\"\n";
	output << "    filter \"configurations:Development\"\n";
	output << "        runtime \"Release\"\n";
	output << "        optimize \"Off\"\n";
	output << "        symbols \"On\"\n";
	output << "    filter \"configurations:Release\"\n";
	output << "        runtime \"Release\"\n";
	output << "        optimize \"On\"\n";
	output << "        symbols \"Off\"\n";
	output << "    filter \"\"\n";
	writeStringList(output, "flags", commonFlags);
	writeStringList(output, "buildoptions", commonBuildOptions);
	writeLuaExpressionList(output, "includedirs", commonIncludePathExpressions);
	writeStringList(output, "defines", commonDefines);
	output << "    objdir (path.join(QFE_PROJECT_ROOT, \"../generated/obj/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}\"))\n";
	output << "    targetdir (path.join(QFE_PROJECT_ROOT, \"../generated/outputs/%{cfg.buildcfg}/%{cfg.platform}\"))\n\n";
	if (!commonDebugDirectoryExpression.empty()) {
		output << "    debugdir (" << commonDebugDirectoryExpression << ")\n\n";
	}

	std::unordered_set<std::uint64_t> groupedNodeIds;
	for (const ProjectGroup& group : groups_) {
		const std::string groupName = Trim(group.name);
		if (groupName.empty()) {
			continue;
		}

		std::vector<const ProjectNode*> members;
		for (const ProjectNode& node : nodes_) {
			if (std::find(group.nodeIds.begin(), group.nodeIds.end(), node.id) !=
				group.nodeIds.end()) {
				members.push_back(&node);
			}
		}
		if (members.empty()) {
			continue;
		}

		output << "group \"" << EscapeLuaString(groupName) << "\"\n";
		for (const ProjectNode* node : members) {
			output << "dofile(path.join(_buildRoot, \"projects/"
				<< EscapeLuaString(generatedProjectDirectories[node->id])
				<< "/premake.lua\"))\n";
			groupedNodeIds.insert(node->id);
		}
		output << "group \"\"\n\n";
	}

	for (const ProjectNode& node : nodes_) {
		if (!groupedNodeIds.contains(node.id)) {
			output << "dofile(path.join(_buildRoot, \"projects/"
				<< EscapeLuaString(generatedProjectDirectories[node.id])
				<< "/premake.lua\"))\n";
		}
	}

	output.flush();
	if (!output) {
		premakeStatus_ = "Failed while writing build/premake5.lua.";
		return;
	}

	const std::filesystem::path readmePath = buildPath / "readme.md";
	std::ofstream readme(readmePath, std::ios::trunc);
	if (!readme) {
		premakeStatus_ = "Could not write build/readme.md.";
		return;
	}
	const std::string rootDirectoryName = rootPath.filename().empty()
		? rootPath.generic_string()
		: QFE::ConvertString(rootPath.filename().wstring());
	const std::filesystem::path parentPath = rootPath.parent_path();
	const std::string parentDirectoryName = parentPath.filename().empty()
		? parentPath.generic_string()
		: QFE::ConvertString(parentPath.filename().wstring());
	readme << "# ProjectGenerator build\n\n";
	readme << "このディレクトリはQuickForgeEngine ProjectGeneratorが生成しました。\n\n";
	readme << "このエディタのルートディレクトリは、`"
		<< parentDirectoryName << "/" << rootDirectoryName
		<< "` に配置してください。\n\n";
	readme << "別の場所へ移動する場合は、`premake5.lua` の "
		<< "`_sourceRootOverride` だけを変更してください。"
		<< "各プロジェクトのPremakeは変更不要です。\n\n";
	readme << "Premakeを実行するときは、この `build` ディレクトリをカレントディレクトリにしてください。\n\n";
	readme << "例:\n\n";
	readme << "```text\n";
	readme << "cd " << rootDirectoryName << "/build\n";
	readme << "premake5 vs2022\n";
	readme << "```\n";
	readme.flush();
	if (!readme) {
		premakeStatus_ = "Failed while writing build/readme.md.";
		return;
	}
	premakeStatus_ = "Generated build Premake: " +
		QFE::ConvertString(buildPath.wstring());
}

void QFE::APPLICATION::ProjectGenerator::SaveConfiguration()
{
	const std::filesystem::path dataDirectory = GetProjectGeneratorDataDirectory();
	std::error_code error;
	std::filesystem::create_directories(dataDirectory, error);
	if (error) {
		premakeStatus_ = "Could not create the ProjectGenerator data directory.";
		return;
	}

	std::wstring selectedConfigurationPath;
	if (!QFE::FRAMEWORK::RequestSaveFilePathFromUser(
		imguiContext_.hwnd,
		L"ProjectGenerator Configuration (*.json)", L"*.json",
		selectedConfigurationPath,
		dataDirectory.wstring(), L"json")) {
		premakeStatus_ = "Save configuration was cancelled.";
		return;
	}
	const std::filesystem::path configurationPath =
		NormalizePath(QFE::ConvertString(selectedConfigurationPath));

	nlohmann::json configuration = nlohmann::json::object();
	configuration["version"] = 4;
	configuration["rootDirectory"] = directoryManager_.GetLootDirectory();
	configuration["nextNodeId"] = nextNodeId_;
	configuration["nextLinkId"] = nextLinkId_;
	configuration["nextGroupId"] = nextGroupId_;
	configuration["commonPremakeSettings"] = {
		{ "workspaceName", commonPremakeSettings_.workspaceName },
		{ "architecture", commonPremakeSettings_.architecture },
		{ "configurations", commonPremakeSettings_.configurations },
		{ "cppDialect", commonPremakeSettings_.cppDialect },
		{ "staticRuntime", commonPremakeSettings_.staticRuntime },
		{ "flags", commonPremakeSettings_.flags },
		{ "buildOptions", commonPremakeSettings_.buildOptions },
		{ "debugDirectory", commonPremakeSettings_.debugDirectory },
		{ "includePaths", commonPremakeSettings_.includePaths },
		{ "defines", commonPremakeSettings_.defines },
	};
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
		nodeJson["externalLinks"] = nlohmann::json::array();
		for (const ProjectNode::ExternalLink& externalLink : node.externalLinks) {
			nodeJson["externalLinks"].push_back({
				{ "filter", externalLink.filter },
				{ "name", externalLink.name },
			});
		}
		nodeJson["libraryDirectories"] = nlohmann::json::array();
		for (const ProjectNode::LibraryDirectory& libraryDirectory :
			node.libraryDirectories) {
			nodeJson["libraryDirectories"].push_back({
				{ "filter", libraryDirectory.filter },
				{ "path", libraryDirectory.path },
			});
		}
		nodeJson["position"] = {
			{ "x", currentPosition.x },
			{ "y", currentPosition.y },
		};
		configuration["nodes"].push_back(std::move(nodeJson));
	}
	configuration["groups"] = nlohmann::json::array();
	for (const ProjectGroup& group : groups_) {
		ImVec2 currentPosition = group.initialPosition;
		ImVec2 currentSize = group.size;
		if (nodeEditorContext_ != nullptr) {
			const ImVec2 editorPosition =
				ax::NodeEditor::GetNodePosition(ToEditorGroupId(group.id));
			if (editorPosition.x != FLT_MAX && editorPosition.y != FLT_MAX) {
				currentPosition = editorPosition;
			}
			const ImVec2 editorSize =
				ax::NodeEditor::GetNodeSize(ToEditorGroupId(group.id));
			if (editorSize.x > 0.0f && editorSize.y > 0.0f) {
				currentSize = editorSize;
			}
		}
		nlohmann::json groupJson = nlohmann::json::object();
		groupJson["id"] = group.id;
		groupJson["name"] = group.name;
		groupJson["nodeIds"] = group.nodeIds;
		groupJson["position"] = {
			{ "x", currentPosition.x },
			{ "y", currentPosition.y },
		};
		groupJson["size"] = {
			{ "x", currentSize.x },
			{ "y", currentSize.y },
		};
		configuration["groups"].push_back(std::move(groupJson));
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
	const std::filesystem::path dataDirectory = GetProjectGeneratorDataDirectory();
	std::error_code directoryError;
	std::filesystem::create_directories(dataDirectory, directoryError);
	if (directoryError) {
		premakeStatus_ = "Could not access the ProjectGenerator data directory.";
		return;
	}

	std::wstring selectedConfigurationPath;
	if (!QFE::FRAMEWORK::RequestGetFilePathFromUser(
		imguiContext_.hwnd,
		L"ProjectGenerator Configuration (*.json)", L"*.json",
		selectedConfigurationPath,
		dataDirectory.wstring())) {
		premakeStatus_ = "Load configuration was cancelled.";
		return;
	}
	const std::filesystem::path configurationPath =
		NormalizePath(QFE::ConvertString(selectedConfigurationPath));
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
	if (configuration.contains("commonPremakeSettings")) {
		if (!configuration["commonPremakeSettings"].is_object()) {
			premakeStatus_ =
				"ProjectGenerator common Premake settings have an invalid format.";
			return;
		}
		try {
			const nlohmann::json& commonSettings =
				configuration["commonPremakeSettings"];
			commonPremakeSettings_.workspaceName = commonSettings.value(
				"workspaceName", commonPremakeSettings_.workspaceName);
			commonPremakeSettings_.architecture = commonSettings.value(
				"architecture", commonPremakeSettings_.architecture);
			commonPremakeSettings_.configurations = commonSettings.value(
				"configurations", commonPremakeSettings_.configurations);
			commonPremakeSettings_.cppDialect = commonSettings.value(
				"cppDialect", commonPremakeSettings_.cppDialect);
			commonPremakeSettings_.staticRuntime = commonSettings.value(
				"staticRuntime", commonPremakeSettings_.staticRuntime);
			commonPremakeSettings_.flags = commonSettings.value(
				"flags", commonPremakeSettings_.flags);
			commonPremakeSettings_.buildOptions = commonSettings.value(
				"buildOptions", commonPremakeSettings_.buildOptions);
			commonPremakeSettings_.debugDirectory = commonSettings.value(
				"debugDirectory", commonPremakeSettings_.debugDirectory);
			commonPremakeSettings_.includePaths = commonSettings.value(
				"includePaths", commonPremakeSettings_.includePaths);
			commonPremakeSettings_.defines = commonSettings.value(
				"defines", commonPremakeSettings_.defines);
		} catch (const nlohmann::json::exception&) {
			premakeStatus_ =
				"ProjectGenerator common Premake settings contain invalid values.";
			return;
		}
	}

	std::string rootLoadStatus;
	const std::string rootDirectory = configuration.value("rootDirectory", "");
	if (!rootDirectory.empty()) {
		const std::filesystem::path rootPath = QFE::ConvertString(rootDirectory);
		std::error_code error;
		if (!std::filesystem::is_directory(rootPath, error)) {
			rootLoadStatus = " Saved root directory was not found.";
		} else if (!directoryManager_.SetLootDirectory(rootDirectory)) {
			rootLoadStatus = " Could not start the saved root directory scan.";
		}
	}

	nodes_.clear();
	links_.clear();
	groups_.clear();
	nextNodeId_ = configuration.value("nextNodeId", std::uint64_t(1));
	nextLinkId_ = configuration.value("nextLinkId", std::uint64_t(1));
	nextGroupId_ = configuration.value("nextGroupId", std::uint64_t(1));
	if (nextNodeId_ == 0) {
		nextNodeId_ = 1;
	}
	if (nextLinkId_ == 0) {
		nextLinkId_ = 1;
	}
	if (nextGroupId_ == 0) {
		nextGroupId_ = 1;
	}
	selectedNodeId_ = 0;
	selectedGroupId_ = 0;
	nodeSettingsOpen_ = false;
	groupSettingsOpen_ = false;

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
			if (nodeJson.contains("externalLinks") &&
				nodeJson["externalLinks"].is_array()) {
				for (const nlohmann::json& externalLinkJson :
					nodeJson["externalLinks"]) {
					const std::string name = externalLinkJson.value("name", "");
					if (!name.empty()) {
						node.externalLinks.push_back({
							externalLinkJson.value("filter", ""), name });
					}
				}
			}
			if (nodeJson.contains("libraryDirectories") &&
				nodeJson["libraryDirectories"].is_array()) {
				for (const nlohmann::json& directoryJson :
					nodeJson["libraryDirectories"]) {
					const std::string path = directoryJson.value("path", "");
					if (!path.empty()) {
						node.libraryDirectories.push_back({
							directoryJson.value("filter", ""), path });
					}
				}
			}
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

	std::unordered_set<std::uint64_t> loadedGroupIds;
	std::unordered_set<std::uint64_t> groupedNodeIds;
	std::size_t invalidGroupCount = 0;
	if (configuration.contains("groups") && configuration["groups"].is_array()) {
		for (const nlohmann::json& groupJson : configuration["groups"]) {
			try {
				ProjectGroup group;
				group.id = groupJson.value("id", std::uint64_t(0));
				group.name = groupJson.value("name", "");
				if (group.id == 0 || !loadedGroupIds.insert(group.id).second) {
					++invalidGroupCount;
					continue;
				}

				if (groupJson.contains("nodeIds") &&
					groupJson["nodeIds"].is_array()) {
					for (const nlohmann::json& nodeIdJson : groupJson["nodeIds"]) {
						const std::uint64_t nodeId = nodeIdJson.get<std::uint64_t>();
						if (knownNodeIds.contains(nodeId) &&
							groupedNodeIds.insert(nodeId).second) {
							group.nodeIds.push_back(nodeId);
						}
					}
				}
				if (group.nodeIds.empty()) {
					++invalidGroupCount;
					continue;
				}

				if (groupJson.contains("position") &&
					groupJson["position"].is_object()) {
					const nlohmann::json& position = groupJson["position"];
					group.initialPosition.x = position.value("x", 0.0f);
					group.initialPosition.y = position.value("y", 0.0f);
				}
				if (groupJson.contains("size") &&
					groupJson["size"].is_object()) {
					const nlohmann::json& size = groupJson["size"];
					group.size.x = std::max(size.value("x", group.size.x), 80.0f);
					group.size.y = std::max(size.value("y", group.size.y), 80.0f);
				}
				group.positionInitialized = false;
				group.needsBoundsUpdate = false;
				const std::uint64_t loadedGroupId = group.id;
				groups_.push_back(std::move(group));

				if (loadedGroupId < std::numeric_limits<std::uint64_t>::max() &&
					nextGroupId_ <= loadedGroupId) {
					nextGroupId_ = loadedGroupId + 1;
				}
			} catch (const nlohmann::json::exception&) {
				++invalidGroupCount;
			}
		}
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
	SimplifyDependencyLinks();

	if (nodeEditorContext_ != nullptr) {
		ax::NodeEditor::SetCurrentEditor(nodeEditorContext_);
		ax::NodeEditor::ClearSelection();
		for (const ProjectNode& node : nodes_) {
			ax::NodeEditor::SetNodePosition(
				ToEditorNodeId(node.id), node.initialPosition);
		}
		for (const ProjectGroup& group : groups_) {
			const ax::NodeEditor::NodeId editorGroupId =
				ToEditorGroupId(group.id);
			ax::NodeEditor::SetNodePosition(
				editorGroupId, group.initialPosition);
			ax::NodeEditor::SetGroupSize(editorGroupId, group.size);
		}
		ax::NodeEditor::SetCurrentEditor(nullptr);
	}

	const std::string configurationPathString =
		QFE::ConvertString(configurationPath.wstring());
	std::ostringstream status;
	status << "Loaded configuration: " << configurationPathString << " ("
		<< nodes_.size() << " node(s), " << links_.size() << " link(s), "
		<< groups_.size() << " group(s)).";
	if (invalidNodeCount != 0 || invalidLinkCount != 0 || invalidGroupCount != 0) {
		status << " Skipped " << invalidNodeCount << " node(s) and "
			<< invalidLinkCount << " link(s), " << invalidGroupCount
			<< " group(s).";
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
