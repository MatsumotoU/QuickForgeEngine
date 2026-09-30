#include "AudioEngine.h"

#define NOMINMAX
#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>
#include <xaudio2.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <limits>
#include <unordered_map>
#include <vector>

#include "string/MyString.h"
#include "EngineDefines.h"

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

namespace {
	using Microsoft::WRL::ComPtr;
	using QFE::AUDIO::AudioCategory;

	constexpr size_t CategoryIndex(AudioCategory category) {
		return static_cast<size_t>(category);
	}

	struct AudioData {
		WAVEFORMATEXTENSIBLE format{};
		std::vector<BYTE> samples;
	};

	std::shared_ptr<AudioData> LoadAudio(const std::wstring& path) {
		ComPtr<IMFSourceReader> reader;
		if (FAILED(MFCreateSourceReaderFromURL(path.c_str(), nullptr, reader.GetAddressOf()))) return {};

		ComPtr<IMFMediaType> requestedType;
		if (FAILED(MFCreateMediaType(requestedType.GetAddressOf())) ||
			FAILED(requestedType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio)) ||
			FAILED(requestedType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM)) ||
			FAILED(reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, requestedType.Get()))) return {};

		ComPtr<IMFMediaType> currentType;
		if (FAILED(reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, currentType.GetAddressOf()))) return {};
		WAVEFORMATEX* waveFormat = nullptr;
		UINT32 formatSize = 0;
		if (FAILED(MFCreateWaveFormatExFromMFMediaType(currentType.Get(), &waveFormat, &formatSize))) return {};
		auto audio = std::make_shared<AudioData>();
		const bool validFormat = formatSize >= sizeof(WAVEFORMATEX) && formatSize <= sizeof(WAVEFORMATEXTENSIBLE);
		if (validFormat) std::memcpy(&audio->format, waveFormat, formatSize);
		CoTaskMemFree(waveFormat);
		if (!validFormat) return {};

		for (;;) {
			ComPtr<IMFSample> sample;
			DWORD flags = 0;
			if (FAILED(reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &flags, nullptr, sample.GetAddressOf()))) return {};
			if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;
			if (!sample) continue; // ストリームの通知など、データがない場合
			ComPtr<IMFMediaBuffer> buffer;
			if (FAILED(sample->ConvertToContiguousBuffer(buffer.GetAddressOf()))) return {};
			BYTE* bytes = nullptr;
			DWORD length = 0;
			if (FAILED(buffer->Lock(&bytes, nullptr, &length))) return {};
			if (length > std::numeric_limits<UINT32>::max() - audio->samples.size()) {
				buffer->Unlock();
				return {};
			}
			audio->samples.insert(audio->samples.end(), bytes, bytes + length);
			buffer->Unlock();
		}
		return audio->samples.empty() ? nullptr : audio;
	}

	struct PlayingVoice {
		IXAudio2SourceVoice* source = nullptr;
		std::shared_ptr<AudioData> audio; // 再生が終わるまで XAudio2 のバッファを保持
		AudioCategory category = AudioCategory::SE;
		float baseVolume = 1.0f;
		bool paused = false;
		~PlayingVoice() {
			if (source) {
				source->Stop();
				source->DestroyVoice();
			}
		}
	};
}

struct QFE::AUDIO::AudioEngine::Impl {
	ComPtr<IXAudio2> xaudio;
	IXAudio2MasteringVoice* master = nullptr;
	bool comInitialized = false;
	bool mediaFoundationInitialized = false;
	uint32_t nextHandle = 1;
	uint32_t nextDataHandle = 1;
	float masterVolume = 1.0f;
	std::array<float, 4> categoryVolumes{ 1.0f, 1.0f, 1.0f, 1.0f };
	std::unordered_map<std::wstring, uint32_t> pathHandles;
	std::unordered_map<uint32_t, std::shared_ptr<AudioData>> loadedData;
	std::unordered_map<uint32_t, std::unique_ptr<PlayingVoice>> voices;
};

QFE::AUDIO::AudioEngine::AudioEngine() : impl_(std::make_unique<Impl>()) {}
QFE::AUDIO::AudioEngine::~AudioEngine() { Shutdown(); }

bool QFE::AUDIO::AudioEngine::Initialize() {
	if (impl_->master) return true;
	const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (SUCCEEDED(comResult)) impl_->comInitialized = true;
	else if (comResult != RPC_E_CHANGED_MODE) return false;
	if (FAILED(MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET))) { Shutdown(); return false; }
	impl_->mediaFoundationInitialized = true;
	if (FAILED(XAudio2Create(impl_->xaudio.GetAddressOf())) ||
		FAILED(impl_->xaudio->CreateMasteringVoice(&impl_->master))) {
		Shutdown();
		return false;
	}
	return true;
}

void QFE::AUDIO::AudioEngine::Shutdown() {
	StopAll();
	impl_->loadedData.clear();
	impl_->pathHandles.clear();
	if (impl_->master) { impl_->master->DestroyVoice(); impl_->master = nullptr; }
	impl_->xaudio.Reset();
	if (impl_->mediaFoundationInitialized) { MFShutdown(); impl_->mediaFoundationInitialized = false; }
	if (impl_->comInitialized) { CoUninitialize(); impl_->comInitialized = false; }
}

uint32_t QFE::AUDIO::AudioEngine::LoadSoundData(const std::string& audioPath) {
	if (!impl_->mediaFoundationInitialized || audioPath.empty()) return 0;
	std::filesystem::path path(QFE::ConvertString(audioPath));
	std::error_code error;
	if (!std::filesystem::exists(path, error) && path.is_relative()) path = std::filesystem::path(L"resources") / path;
	path = std::filesystem::absolute(path, error);
	if (error || !std::filesystem::exists(path, error)) {
		QFE_LOG("Audio file not found: " + audioPath);
		return 0;
	}
	const std::wstring cacheKey = path.wstring();
	const auto cached = impl_->pathHandles.find(cacheKey);
	if (cached != impl_->pathHandles.end()) return cached->second;
	const std::shared_ptr<AudioData> audio = LoadAudio(cacheKey);
	if (!audio) { QFE_LOG("Failed to load audio: " + audioPath); return 0; }
	uint32_t handle = impl_->nextDataHandle++;
	if (handle == 0) handle = impl_->nextDataHandle++;
	impl_->loadedData.emplace(handle, audio);
	impl_->pathHandles.emplace(cacheKey, handle);
	return handle;
}

uint32_t QFE::AUDIO::AudioEngine::PlaySoundForAudioData(
	uint32_t audioDataHandle, bool loop, float volume, AudioCategory category) {
	if (!impl_->master) return 0;
	const auto data = impl_->loadedData.find(audioDataHandle);
	if (data == impl_->loadedData.end() || CategoryIndex(category) >= impl_->categoryVolumes.size()) return 0;
	const std::shared_ptr<AudioData>& audio = data->second;

	auto voice = std::make_unique<PlayingVoice>();
	voice->audio = audio;
	voice->category = category;
	voice->baseVolume = std::clamp(volume, 0.0f, 1.0f);
	if (FAILED(impl_->xaudio->CreateSourceVoice(&voice->source, &audio->format.Format))) return 0;
	XAUDIO2_BUFFER buffer{};
	buffer.AudioBytes = static_cast<UINT32>(audio->samples.size());
	buffer.pAudioData = audio->samples.data();
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
	if (FAILED(voice->source->SubmitSourceBuffer(&buffer)) ||
		FAILED(voice->source->SetVolume(voice->baseVolume * impl_->categoryVolumes[CategoryIndex(category)])) ||
		FAILED(voice->source->Start())) return 0;
	uint32_t handle = impl_->nextHandle++;
	if (handle == 0) handle = impl_->nextHandle++;
	impl_->voices.emplace(handle, std::move(voice));
	return handle;
}

uint32_t QFE::AUDIO::AudioEngine::Play(
	const std::string& audioPath, bool loop, float volume, AudioCategory category) {
	return PlaySoundForAudioData(LoadSoundData(audioPath), loop, volume, category);
}

void QFE::AUDIO::AudioEngine::Stop(uint32_t handle) { impl_->voices.erase(handle); }
void QFE::AUDIO::AudioEngine::Pause(uint32_t handle) {
	const auto voice = impl_->voices.find(handle);
	if (voice != impl_->voices.end() && !voice->second->paused && SUCCEEDED(voice->second->source->Stop())) {
		voice->second->paused = true;
	}
}
void QFE::AUDIO::AudioEngine::Resume(uint32_t handle) {
	const auto voice = impl_->voices.find(handle);
	if (voice != impl_->voices.end() && voice->second->paused && SUCCEEDED(voice->second->source->Start())) {
		voice->second->paused = false;
	}
}
void QFE::AUDIO::AudioEngine::StopAll() { impl_->voices.clear(); }
void QFE::AUDIO::AudioEngine::PauseAll() {
	for (const auto& entry : impl_->voices) Pause(entry.first);
}
void QFE::AUDIO::AudioEngine::ResumeAll() {
	for (const auto& entry : impl_->voices) Resume(entry.first);
}
void QFE::AUDIO::AudioEngine::SetMasterVolume(float volume) {
	const float clamped = std::clamp(volume, 0.0f, 1.0f);
	if (impl_->master && SUCCEEDED(impl_->master->SetVolume(clamped))) impl_->masterVolume = clamped;
}
float QFE::AUDIO::AudioEngine::GetMasterVolume() const { return impl_->masterVolume; }
void QFE::AUDIO::AudioEngine::SetCategoryVolume(AudioCategory category, float volume) {
	if (CategoryIndex(category) >= impl_->categoryVolumes.size()) return;
	impl_->categoryVolumes[CategoryIndex(category)] = std::clamp(volume, 0.0f, 1.0f);
	for (const auto& [handle, voice] : impl_->voices) {
		if (voice->category == category) {
			voice->source->SetVolume(voice->baseVolume * impl_->categoryVolumes[CategoryIndex(category)]);
		}
	}
}
float QFE::AUDIO::AudioEngine::GetCategoryVolume(AudioCategory category) const {
	return CategoryIndex(category) < impl_->categoryVolumes.size()
		? impl_->categoryVolumes[CategoryIndex(category)] : 0.0f;
}
void QFE::AUDIO::AudioEngine::SetVolume(uint32_t handle, float volume) {
	const auto voice = impl_->voices.find(handle);
	if (voice != impl_->voices.end()) {
		voice->second->baseVolume = std::clamp(volume, 0.0f, 1.0f);
		voice->second->source->SetVolume(voice->second->baseVolume *
			impl_->categoryVolumes[CategoryIndex(voice->second->category)]);
	}
}

void QFE::AUDIO::AudioEngine::Update() {
	for (auto it = impl_->voices.begin(); it != impl_->voices.end();) {
		XAUDIO2_VOICE_STATE state{};
		it->second->source->GetState(&state);
		if (state.BuffersQueued == 0) it = impl_->voices.erase(it);
		else ++it;
	}
}
