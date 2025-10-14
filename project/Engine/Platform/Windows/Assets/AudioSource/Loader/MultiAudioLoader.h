#pragma once
#include <string>
#include <wrl.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

struct AudioData;

class MultiAudioLoader final{
public:
	~MultiAudioLoader();

public:
	void Initialize();

};

namespace Multiaudioloader {
	AudioData LoadAudioData(const std::string& path);
}