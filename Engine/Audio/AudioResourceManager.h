#pragma once
#include <string>
#include <vector>
#include <xaudio2.h>
#include "SoundData.h"
#include "String/StringLiblary.h"

class EngineCore;

class AudioResourceManager {
public:
	~AudioResourceManager();

public:
	/// <summary>
	/// 音声データ管理システムを初期化します
	/// </summary>
	/// <param name="engineCore"></param>
	void Initialize(EngineCore* engineCore);
	/// <summary>
	/// ゲームに使用できる音声データを音声ファイルから作成します
	/// </summary>
	/// <param name="filePath">FilePath/</param>
	/// <param name="fileName">FileName</param>
	/// <returns></returns>
	bool LoadAudioResource(const std::string& filePath , const std::string& fileName);
	bool LoadAudioResource(const std::string& filePath);
	/// <summary>
	/// 音声データの管理番号を取得します
	/// </summary>
	/// <param name="fileName">FileName.mp3</param>
	/// <returns></returns>
	uint32_t GetAudioDataHandle(const std::string& fileName);
	/// <summary>
	/// 音声データの管理番号から音声データを取得します
	/// </summary>
	/// <param name="audioDataHandle"></param>
	/// <returns></returns>
	SoundData GetSoundData(uint32_t audioDataHandle);
	/// <summary>
	/// 音声データを直接登録します
	/// </summary>
	/// <param name="soundData"></param>
	/// <param name="fileName"></param>
	void AddSoundData(const SoundData& soundData, const std::string& fileName);

public:// デバッグ機能
#ifdef _DEBUG
	void DrawImGui();
	void DisplayWaveform(const std::string& plotName,const SoundData& data);
#endif // _DEBUG

private:
	EngineCore* engineCore_;
	StringLiblary fileNameLiblary_;
	std::vector<SoundData> soundData_;

private:
#ifdef _DEBUG
	int32_t debugResourceIndex_;
#endif // _DEBUG
};