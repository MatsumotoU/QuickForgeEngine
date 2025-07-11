#pragma once
#include <string>

enum FileExtension {
	FileExtension_None,
	FileExtension_PNG,
	FileExtension_JPG,
	FileExtension_WAV,
	FileExtension_MP3,
};

FileExtension GetFileExtension(const std::string& fileName);