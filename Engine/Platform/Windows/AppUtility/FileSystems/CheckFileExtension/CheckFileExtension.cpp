#include "CheckFileExtension.h"

bool CheckFileExtension::HasExtension(const std::string& fileName, const std::string& extension) {
	if (fileName.empty() || extension.empty()) {
		return false;
	}
	return fileName.size() >= extension.size() &&
		fileName.compare(fileName.size() - extension.size(), extension.size(), extension) == 0;
}
