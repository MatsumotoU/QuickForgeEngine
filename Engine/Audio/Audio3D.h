#pragma once
#include <xaudio2.h>
#include <x3daudio.h>
#pragma comment(lib,"xaudio2.lib")
#include "SoundData.h"

class Audio3D {

public:
	void Initialize(IXAudio2MasteringVoice* pMasterVoice);

public:
	X3DAUDIO_DSP_SETTINGS CreateDspSettings(X3DAUDIO_LISTENER* listener,X3DAUDIO_EMITTER* emitter, const SoundData& soundData);

public:
	X3DAUDIO_HANDLE* GetX3DInstance();

private:
	X3DAUDIO_HANDLE x3DInstance_;
};

namespace audio3d {

	IXAudio2SourceVoice* Create3DSourceVoice(IXAudio2* xAudio2, IXAudio2MasteringVoice* pMasterVoice, const SoundData& soundData,const X3DAUDIO_DSP_SETTINGS& dspSettings);
}