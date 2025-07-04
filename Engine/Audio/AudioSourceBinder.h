#pragma once
#include <xaudio2.h>
#include <map>
#include <string>

class EngineCore;

class AudioSourceBinder {
public:
	~AudioSourceBinder();

public:// 一度は呼ばないといけないやつ
	void Initialize(EngineCore* engineCore);

public:// メイン機能
	/// <summary>
	/// 音声データからソースボイスを作成して管理名と結びつけます
	/// </summary>
	/// <param name="sourceVoiceFriendryName"></param>
	/// <param name="audioHandle"></param>
	void CreateSourceVoice(const std::string& sourceVoiceFriendryName, uint32_t audioHandle);
	/// <summary>
	/// 管理名からソースボイスを取得します
	/// </summary>
	/// <param name="sourceVoiceFriendryName"></param>
	/// <returns></returns>
	IXAudio2SourceVoice* GetSourceVoice(const std::string& sourceVoiceFriendryName);

public:
#ifdef _DEBUG
	void DrawImGui();
#endif // _DEBUG

public:
	uint32_t GetSourceVoiceMapNumber();
	std::string GetsourceVoiceMapElement(uint32_t index);

private:
	EngineCore* engineCore_;
	std::map<std::string, IXAudio2SourceVoice*> sourceVoiceMap_;

};