#pragma once
#define XAUDIO2_DEBUG 1

#include <wrl.h>
#include <xaudio2.h>
#include <cstdint>
namespace QFE {
	class XAudioCore final {
	public:
		~XAudioCore();

	public:
		/// xAudio2を作�EしまぁE
		void Initialize();
		/// xAudio2を終亁E��まぁE
		void Finalize();

	public:
		/// 全体�E音量を調節しまぁE1.0が標溁E
		void SetMasterVolume(float volume);
		uint32_t GetOutputChannels();
		IXAudio2MasteringVoice* GetMasterVoice();
		IXAudio2* GetXAudio2();

	private:
		Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
		IXAudio2MasteringVoice* masterVoice_;
	};
}