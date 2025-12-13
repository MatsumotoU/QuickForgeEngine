#pragma once
#include <string>
#include <wrl.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

struct AudioData;

namespace Multiaudioloader {
	void Initialize();
	void Finalize();
	AudioData LoadAudioData(const std::string& path);
}
