#include "Chiptune.h"
#include <cmath>
#include <cstring>
#include <random>

#include "Base/EngineCore.h"
#include "AudioPlayer.h"

// WAVEFORMATEXの設定パラメータ
constexpr int sampleRate = 44100;      // サンプリングレート
constexpr float durationSec = 0.1f;    // 秒数
constexpr int frequency = 440;         // 矩形波の周波数(Hz)
constexpr int amplitude = 30000;       // 振幅（16bit PCMの最大値は32767）
constexpr int channels = 1;            // モノラル
constexpr int bitsPerSample = 16;      // 16bit PCM
int sampleCount = static_cast<int>(static_cast<float> (sampleRate) * durationSec);
int bufferSize = sampleCount * channels * (bitsPerSample / 8);

Chiptune::~Chiptune() {
    
}

void Chiptune::Initialize(EngineCore* enginecore) {
    mainSquareWaveSoureceVoiceName_ = "MainSquareWave";
    subSquareWaveSoureceVoiceName_ = "SubSquareWave";
    triangleWaveSoureceVoiceName_ = "TriangleWave";
    noiseSoureceVoiceName_ = "Noise";

    engineCore_ = enginecore;
    audioPlayer_ = enginecore->GetAudioPlayer();
    // 原音抽出
    AudioResourceManager* audioResourceManager = engineCore_->GetAudioResourceManager();
    audioResourceManager->AddSoundData(GenerateSquareWave(),"SquareWave");
    audioResourceManager->AddSoundData(GenerateTriangleWave(), "TriangleWave");
    audioResourceManager->AddSoundData(GenerateNoise(), "Noise");
    // 原音ハンドルを取得
    squareAudioHandle_ = audioResourceManager->GetAudioDataHandle("SquareWave");
    triangleAudioHandle_ = audioResourceManager->GetAudioDataHandle("TriangleWave");
    noiseAudioHandle_ = audioResourceManager->GetAudioDataHandle("Noise");

    // 音を生成
    engineCore_->GetAudioSourceBinder()->CreateSourceVoice(mainSquareWaveSoureceVoiceName_, squareAudioHandle_);
    engineCore_->GetAudioSourceBinder()->CreateSourceVoice(subSquareWaveSoureceVoiceName_, squareAudioHandle_);
    engineCore_->GetAudioSourceBinder()->CreateSourceVoice(triangleWaveSoureceVoiceName_, triangleAudioHandle_);
    engineCore_->GetAudioSourceBinder()->CreateSourceVoice(noiseSoureceVoiceName_, noiseAudioHandle_);
}

void Chiptune::PlayMainSquareWave() {
    audioPlayer_->PlayAudio(squareAudioHandle_, mainSquareWaveSoureceVoiceName_,false);
}

void Chiptune::PlaySubSquareWave() {
    audioPlayer_->PlayAudio(squareAudioHandle_, subSquareWaveSoureceVoiceName_,false);
}

void Chiptune::PlayTriangleWave() {
    audioPlayer_->PlayAudio(triangleAudioHandle_, triangleWaveSoureceVoiceName_, false);
}

void Chiptune::PlayNoise() {
    audioPlayer_->PlayAudio(noiseAudioHandle_, noiseSoureceVoiceName_, false);
}

void Chiptune::PlayMainSquareWave(const GermanNote& note, uint32_t octave, float second) {
    if (second <= 0.1f) {
        audioPlayer_->PlayAudio(squareAudioHandle_, mainSquareWaveSoureceVoiceName_, false);
        return;
    }

    float rate = GetFrequencyFormGermanNote(note, octave) / GetFrequencyFormGermanNote(GermanNote::A, 4);
    engineCore_->GetAudioSourceBinder()->GetSourceVoice(mainSquareWaveSoureceVoiceName_)->SetFrequencyRatio(rate);

    uint32_t loopCount = static_cast<uint32_t>((second * 10.0f) * rate);
    audioPlayer_->PlayAudio(squareAudioHandle_, mainSquareWaveSoureceVoiceName_, 0.0f, 0.1f, loopCount);
}

void Chiptune::PlaySubSquareWave(const GermanNote& note, uint32_t octave, float second) {
    if (second <= 0.1f) {
        audioPlayer_->PlayAudio(squareAudioHandle_, subSquareWaveSoureceVoiceName_, false);
        return;
    }

    float rate = GetFrequencyFormGermanNote(note, octave) / GetFrequencyFormGermanNote(GermanNote::A, 4);
    engineCore_->GetAudioSourceBinder()->GetSourceVoice(subSquareWaveSoureceVoiceName_)->SetFrequencyRatio(rate);

    uint32_t loopCount = static_cast<uint32_t>((second * 10.0f) * rate);
    audioPlayer_->PlayAudio(squareAudioHandle_, subSquareWaveSoureceVoiceName_, 0.0f, 0.1f, loopCount);
}

void Chiptune::PlayTriangWave(const GermanNote& note, uint32_t octave, float second) {
    if (second <= 0.1f) {
        audioPlayer_->PlayAudio(triangleAudioHandle_, triangleWaveSoureceVoiceName_, false);
        return;
    }

    float rate = GetFrequencyFormGermanNote(note, octave) / GetFrequencyFormGermanNote(GermanNote::A, 4);
    engineCore_->GetAudioSourceBinder()->GetSourceVoice(triangleWaveSoureceVoiceName_)->SetFrequencyRatio(rate);

    uint32_t loopCount = static_cast<uint32_t>((second * 10.0f) * rate);
    audioPlayer_->PlayAudio(triangleAudioHandle_, triangleWaveSoureceVoiceName_, 0.0f, 0.1f, loopCount);
}

void Chiptune::PlayNoise(float second) {
    if (second <= 0.1f) {
        audioPlayer_->PlayAudio(noiseAudioHandle_, noiseSoureceVoiceName_, false);
        return;
    }

    uint32_t loopCount = static_cast<uint32_t>(second * 10.0f);
    audioPlayer_->PlayAudio(noiseAudioHandle_, noiseSoureceVoiceName_, 0.0f, 0.1f, loopCount);
}

bool Chiptune::GetIsPlayMainSquareWave() {
    return Audiomanager::GetIsPlaying(engineCore_->GetAudioSourceBinder()->GetSourceVoice(mainSquareWaveSoureceVoiceName_));
}

bool Chiptune::GetIsPlaySubSquareWave() {
    return Audiomanager::GetIsPlaying(engineCore_->GetAudioSourceBinder()->GetSourceVoice(subSquareWaveSoureceVoiceName_));
}

bool Chiptune::GetIsPlayTriangleWave() {
    return Audiomanager::GetIsPlaying(engineCore_->GetAudioSourceBinder()->GetSourceVoice(triangleWaveSoureceVoiceName_));
}

bool Chiptune::GetIsPlayNoise() {
    return Audiomanager::GetIsPlaying(engineCore_->GetAudioSourceBinder()->GetSourceVoice(noiseSoureceVoiceName_));
}

SoundData Chiptune::GenerateBaseSoundData() {
    SoundData result{};
    // WAVEFORMATEXの設定
    std::memset(&result.wfex, 0, sizeof(WAVEFORMATEX));
    result.wfex.wFormatTag = WAVE_FORMAT_PCM;
    result.wfex.nChannels = channels;
    result.wfex.nSamplesPerSec = sampleRate;
    result.wfex.wBitsPerSample = bitsPerSample;
    result.wfex.nBlockAlign = channels * (bitsPerSample / 8);
    result.wfex.nAvgBytesPerSec = sampleRate * result.wfex.nBlockAlign;

    // バッファ確保
    result.pBuffer = new BYTE[bufferSize];
    result.bufferSize = bufferSize;
    return result;
}

SoundData Chiptune::GenerateSquareWave() {
    SoundData result = GenerateBaseSoundData();
    // 矩形波生成
    for (int i = 0; i < sampleCount; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        // 1周期の前半を+amplitude、後半を-amplitude
        short value = ((std::fmod(t * frequency, 1.0) < 0.5) ? amplitude : -amplitude);
        // 16bit little endianで書き込む
        result.pBuffer[i * 2 + 0] = value & 0xFF;
        result.pBuffer[i * 2 + 1] = (value >> 8) & 0xFF;
    }
    return result;
}

SoundData Chiptune::GenerateTriangleWave() {
    SoundData result = GenerateBaseSoundData();

    for (int i = 0; i < sampleCount; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        // 三角波の式: 4A * |(t/T - floor(t/T + 0.5))| - A
        double period = 1.0 / frequency;
        double phase = std::fmod(t, period) / period;
        double tri = 4.0 * amplitude * std::abs(phase - 0.5) - amplitude;
        short value = static_cast<short>(tri);

        // 16bit little endianで書き込む
        result.pBuffer[i * 2 + 0] = value & 0xFF;
        result.pBuffer[i * 2 + 1] = (value >> 8) & 0xFF;
    }

    return result;
}

SoundData Chiptune::GenerateNoise() {
    SoundData result = GenerateBaseSoundData();

    // 乱数生成器
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<short> dist(-30000, 30000);

    for (int i = 0; i < sampleCount; ++i) {
        short value = dist(rng);
        // 16bit little endianで書き込む
        result.pBuffer[i * 2 + 0] = value & 0xFF;
        result.pBuffer[i * 2 + 1] = (value >> 8) & 0xFF;
    }

    return result;
}
