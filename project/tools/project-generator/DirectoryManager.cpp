#include "DirectoryManager.h"

#include "core/string/MyString.h"

#define NOMINMAX
#include <Windows.h>

#include <algorithm>
#include <chrono>
#include <exception>
#include <system_error>
#include <utility>

namespace
{
	using DirectoryEntry = QFE::APPLICATION::DirectoryEntry;

	bool IsReady(const std::future<std::vector<DirectoryEntry>>& scan)
	{
		return scan.valid() &&
			scan.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
	}

	bool IsHiddenPath(const std::filesystem::path& path)
	{
		const DWORD attributes = GetFileAttributesW(path.c_str());
		return attributes != INVALID_FILE_ATTRIBUTES &&
			(attributes & FILE_ATTRIBUTE_HIDDEN) != 0;
	}
}

void QFE::APPLICATION::DirectoryManager::Initialize()
{
	lootDirectory_.clear();
	directories_.clear();
	scanState_ = DirectoryScanState::Idle;
}

bool QFE::APPLICATION::DirectoryManager::SetLootDirectory(const std::string& path)
{
	Update();
	if (IsScanRunning() || path.empty()) {
		return false;
	}

	const std::filesystem::path rootPath = QFE::ConvertString(path);
	std::error_code error;
	if (!std::filesystem::is_directory(rootPath, error)) {
		scanState_ = DirectoryScanState::Failed;
		return false;
	}

	lootDirectory_ = path;
	directories_.clear();
	scanState_ = DirectoryScanState::Scanning;

	try {
		directoryScan_ = std::async(
			std::launch::async,
			[rootPath]() { return ScanDirectoryTree(rootPath); });
	} catch (const std::exception&) {
		scanState_ = DirectoryScanState::Failed;
		return false;
	}

	return true;
}

void QFE::APPLICATION::DirectoryManager::Update()
{
	if (!IsReady(directoryScan_)) {
		return;
	}

	try {
		directories_ = directoryScan_.get();
		scanState_ = DirectoryScanState::Ready;
	} catch (const std::exception&) {
		directories_.clear();
		scanState_ = DirectoryScanState::Failed;
	}
}

const std::string& QFE::APPLICATION::DirectoryManager::GetLootDirectory() const
{
	return lootDirectory_;
}

const std::vector<QFE::APPLICATION::DirectoryEntry>&
QFE::APPLICATION::DirectoryManager::GetDirectories() const
{
	return directories_;
}

QFE::APPLICATION::DirectoryScanState
QFE::APPLICATION::DirectoryManager::GetScanState() const
{
	return scanState_;
}

bool QFE::APPLICATION::DirectoryManager::IsScanRunning() const
{
	return directoryScan_.valid() && !IsReady(directoryScan_);
}

std::vector<QFE::APPLICATION::DirectoryEntry>
QFE::APPLICATION::DirectoryManager::ScanDirectoryTree(
	const std::filesystem::path& rootPath)
{
	std::vector<DirectoryEntry> directories;
	std::error_code error;
	std::filesystem::recursive_directory_iterator iterator(
		rootPath,
		std::filesystem::directory_options::skip_permission_denied,
		error);
	const std::filesystem::recursive_directory_iterator end;

	while (!error && iterator != end) {
		if (IsHiddenPath(iterator->path())) {
			// 隠しディレクトリは一覧に出さず、その配下も走査しない。
			iterator.disable_recursion_pending();
			error.clear();
			iterator.increment(error);
			continue;
		}

		std::error_code entryError;
		if (iterator->is_directory(entryError)) {
			const std::filesystem::path absolutePath = iterator->path();
			DirectoryEntry entry;
			entry.absolutePath = absolutePath;
			entry.relativePath = absolutePath.lexically_relative(rootPath);
			entry.name = QFE::ConvertString(absolutePath.filename().wstring());
			entry.depth = static_cast<std::uint32_t>(iterator.depth());
			directories.emplace_back(std::move(entry));
		}

		error.clear();
		iterator.increment(error);
	}

	std::sort(
		directories.begin(),
		directories.end(),
		[](const DirectoryEntry& left, const DirectoryEntry& right) {
			return left.relativePath.generic_string() < right.relativePath.generic_string();
		});

	return directories;
}
