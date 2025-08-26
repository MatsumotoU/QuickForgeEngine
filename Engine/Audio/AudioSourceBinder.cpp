#include "AudioSourceBinder.h"
#include "Base/EngineCore.h"

#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
#include "Base/DirectX/ImGuiManager.h"
#endif // _DEBUG
#include <cassert>

AudioSourceBinder::~AudioSourceBinder() {
	// 登録されている物の表示
#ifdef _DEBUG
	DebugLog("=====AudioSource=====");
	uint32_t count = 0;
	for (const std::pair<std::string, SoruceVoiceData>& pair : sourceVoiceMap_) {
		DebugLog(std::format("Key[{}]: {} ",count, pair.first));
		count++;
	}
	DebugLog("=====================");
#endif // _DEBUG

	// ソースボイスの削除
	for (const std::pair<std::string, SoruceVoiceData>& pair : sourceVoiceMap_) {
		pair.second.sourceVoice->DestroyVoice();
	}
	sourceVoiceMap_.clear();
}

void AudioSourceBinder::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	sourceVoiceMap_.clear();
}

void AudioSourceBinder::CreateSourceVoice(const std::string& sourceVoiceFriendryName, uint32_t audioHandle) {
	SoruceVoiceData result{};

	result.audioHandle = audioHandle;
	// 音声データからソースボイス作成
	result.sourceVoice = Audiomanager::CreateSourceVoice(
		engineCore_->GetAudioManager()->xAudio2_.Get(),
		engineCore_->GetAudioResourceManager()->GetSoundData(audioHandle));
	// ソースボイスと管理名でバインド
	sourceVoiceMap_.emplace(sourceVoiceFriendryName, result);
}

IXAudio2SourceVoice* AudioSourceBinder::GetSourceVoice(const std::string& sourceVoiceFriendryName) {
	// イテレータを取得して要素があるか探索
	std::map<std::string, SoruceVoiceData>::iterator it = sourceVoiceMap_.find(sourceVoiceFriendryName);
	assert(it != sourceVoiceMap_.end());
#ifdef _DEBUG
	DebugLog(std::format("Returen: {}", it->first));
#endif // _DEBUG
	// バインドされたソースボイスを返す
	return it->second.sourceVoice;
}

#ifdef _DEBUG
void AudioSourceBinder::DrawImGui() {
	uint32_t count = 0;
	std::string result{};
	for (const std::pair<std::string, SoruceVoiceData>& pair : sourceVoiceMap_) {
		if (ImGui::TreeNode(pair.first.c_str())) {
			if (ImGui::Button("Play")) {
				Audiomanager::SoundPlaySourceVoice(
					engineCore_->GetAudioResourceManager()->GetSoundData(pair.second.audioHandle),
					pair.second.sourceVoice,false);
			}

			ImGui::Text("IsPlaying: %s", Audiomanager::GetIsPlaying(pair.second.sourceVoice) ? "Ture" : "False");
			float volume{};
			pair.second.sourceVoice->GetVolume(&volume);
			ImGui::Text("Volume: %f", volume);
			float frequency{};
			pair.second.sourceVoice->GetFrequencyRatio(&frequency);
			ImGui::Text("Frequency: %f", frequency);
			ImGui::TreePop();
		}
		count++;
	}
	
}
#endif // _DEBUG

uint32_t AudioSourceBinder::GetSourceVoiceMapNumber() {
	return static_cast<uint32_t>(sourceVoiceMap_.size());
}

std::string AudioSourceBinder::GetsourceVoiceMapElement(uint32_t index) {
	uint32_t count = 0;
	std::string result{};
	for (const std::pair<std::string, SoruceVoiceData>& pair : sourceVoiceMap_) {
		if (index == count) {
			result = pair.first;
		}
		count++;
	}
	//assert(result != "");
	return result;
}
