#include "AudioSourceBinder.h"
#include "Base/EngineCore.h"

#ifdef _DEBUG
#include "Base/MyDebugLog.h"
#include "Base/DirectX/ImGuiManager.h"
#endif // _DEBUG
#include <cassert>

AudioSourceBinder::~AudioSourceBinder() {
	// 登録されている物の表示
#ifdef _DEBUG
	DebugLog("=====AudioSource=====");
	uint32_t count = 0;
	for (const std::pair<std::string, IXAudio2SourceVoice*>& pair : sourceVoiceMap_) {
		DebugLog(std::format("Key[{}]: {} ",count, pair.first));
		count++;
	}
	DebugLog("=====================");
#endif // _DEBUG

	// ソースボイスの削除
	for (const std::pair<std::string, IXAudio2SourceVoice*>& pair : sourceVoiceMap_) {
		pair.second->DestroyVoice();
	}
	sourceVoiceMap_.clear();
}

void AudioSourceBinder::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	sourceVoiceMap_.clear();
}

void AudioSourceBinder::CreateSourceVoice(const std::string& sourceVoiceFriendryName, uint32_t audioHandle) {
	IXAudio2SourceVoice* result = nullptr;

	// 音声データからソースボイス作成
	result = Audiomanager::CreateSourceVoice(
		engineCore_->GetAudioManager()->xAudio2_.Get(),
		engineCore_->GetAudioResourceManager()->GetSoundData(audioHandle));
	// ソースボイスと管理名でバインド
	sourceVoiceMap_.emplace(sourceVoiceFriendryName, result);
}

IXAudio2SourceVoice* AudioSourceBinder::GetSourceVoice(const std::string& sourceVoiceFriendryName) {
	// イテレータを取得して要素があるか探索
	std::map<std::string, IXAudio2SourceVoice*>::iterator it = sourceVoiceMap_.find(sourceVoiceFriendryName);
	assert(it != sourceVoiceMap_.end());
#ifdef _DEBUG
	DebugLog(std::format("Returen: {}", it->first));
#endif // _DEBUG
	// バインドされたソースボイスを返す
	return it->second;
}

#ifdef _DEBUG
void AudioSourceBinder::DrawImGui() {
	uint32_t count = 0;
	std::string result{};
	for (const std::pair<std::string, IXAudio2SourceVoice*>& pair : sourceVoiceMap_) {
		if (ImGui::TreeNode(pair.first.c_str())) {
			ImGui::Text("IsPlaying: %s", Audiomanager::GetIsPlaying(pair.second) ? "Ture" : "False");
			float volume{};
			pair.second->GetVolume(&volume);
			ImGui::Text("Volume: %f", volume);
			float frequency{};
			pair.second->GetFrequencyRatio(&frequency);
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
	for (const std::pair<std::string, IXAudio2SourceVoice*>& pair : sourceVoiceMap_) {
		if (index == count) {
			result = pair.first;
		}
		count++;
	}
	//assert(result != "");
	return result;
}
