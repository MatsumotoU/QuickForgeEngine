#pragma once
#include <string>
#include <xaudio2.h>
#include "SoundData.h"
#include "MusicalScale.h"

class EngineCore;
class AudioPlayer;

class Chiptune {
public:
	~Chiptune();

public:// 一度は通らないとバグる関数
	void Initialize(EngineCore* enginecore);

public:// 短音
	/// <summary>
	/// A4のメイン矩形波を0.1秒再生します
	/// </summary>
	void PlayMainSquareWave();
	/// <summary>
	/// A4のサブ矩形波を0.1秒再生します
	/// </summary>
	void PlaySubSquareWave();
	/// <summary>
	/// A4の三角波を0.1秒再生します
	/// </summary>
	void PlayTriangleWave();
	/// <summary>
	/// ノイズを0.1秒再生します
	/// </summary>
	void PlayNoise();

public:// 指定
	/// <summary>
	/// 音階と時間を指定して矩形波を鳴らします
	/// </summary>
	/// <param name="note">ドレミ</param>
	/// <param name="octave">オクターブ(4が安定)</param>
	/// <param name="second">再生時間</param>
	void PlayMainSquareWave(const GermanNote& note,uint32_t octave,float second,float volume);
	void PlaySubSquareWave(const GermanNote& note, uint32_t octave, float second, float volume);
	void PlayTriangWave(const GermanNote& note, uint32_t octave, float second, float volume);
	void PlayNoise(float second, float volume);

public:// ゲッター
	bool GetIsPlayMainSquareWave();
	bool GetIsPlaySubSquareWave();
	bool GetIsPlayTriangleWave();
	bool GetIsPlayNoise();

private:// 内部機能
	/// <summary>
	/// WAVEFORMATEXの設定が終わっている空の音声データを返します
	/// </summary>
	/// <returns></returns>
	SoundData GenerateBaseSoundData();
	/// <summary>
	/// 矩形波を生成します
	/// </summary>
	/// <returns></returns>
	SoundData GenerateSquareWave();
	/// <summary>
	/// 三角波を生成します
	/// </summary>
	/// <returns></returns>
	SoundData GenerateTriangleWave();
	/// <summary>
	/// ノイズを生成します
	/// </summary>
	/// <returns></returns>
	SoundData GenerateNoise();

public:
	std::string mainSquareWaveSoureceVoiceName_;
	std::string subSquareWaveSoureceVoiceName_;
	std::string triangleWaveSoureceVoiceName_;
	std::string noiseSoureceVoiceName_;

	uint32_t squareAudioHandle_;
	uint32_t triangleAudioHandle_;
	uint32_t noiseAudioHandle_;

private:
	EngineCore* engineCore_;
	AudioPlayer* audioPlayer_;
};