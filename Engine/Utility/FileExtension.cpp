#include "FileExtension.h"

FileExtension GetFileExtension(const std::string& fileName) {
	std::string result = fileName.substr(fileName.find_last_of('.') + 1);
	if (result == "png") {
		return FileExtension_PNG;
	} else if (result == "jpg" || result == "jpeg") {
		return FileExtension_JPG;
	} else if (result == "wav") {
		return FileExtension_WAV;
	} else if (result == "mp3") {
		return FileExtension_MP3;
	}
	return FileExtension_None;
}
