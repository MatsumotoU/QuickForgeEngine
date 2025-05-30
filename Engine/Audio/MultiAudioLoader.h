#pragma once
#include <string>
#include <wrl.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

struct SoundData;
class AudioManager;

// Mp3ファイル糞音小さい

class MultiAudioLoader {
public:
	~MultiAudioLoader();

public:
	void Initialize();

};

namespace Multiaudioloader {
	SoundData LoadSoundData(const std::string& path);
}