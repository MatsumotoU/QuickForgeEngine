#pragma once
#include <string>
#include <wrl.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

struct SoundData;
class XAudioCore;

class MultiAudioLoader final{
public:
	~MultiAudioLoader();

public:
	void Initialize();

};

namespace Multiaudioloader {
	SoundData LoadSoundData(const std::string& path);
}