#include "AudioMath.h"
#include "Assets/AudioSource/Data/AudioData.h"
#include "Assets/AudioSource/Data/Spectrum.h"

// DFTに必要な標準ライブラリ




float MyAudioMath::SpeedOfSoundFromTemperature(float celsius) {
    return 331.5f + 0.6f * celsius;
}

Spectrum MyAudioMath::CreateSpectrumFromAudioData(const AudioData& audioData) {
    Spectrum result;

    // 16bit PCM monoのみ対応
    if (audioData.wfex.wFormatTag != WAVE_FORMAT_PCM || audioData.wfex.wBitsPerSample != 16) {
        throw std::runtime_error("Unsupported audio format for FFT calculation: Only 16-bit PCM mono is supported.");
    }

    const auto* samples = reinterpret_cast<const int16_t*>(audioData.pBuffer);
    size_t numSamples = audioData.bufferSize / sizeof(int16_t);

    if (numSamples == 0) {
        return result;
    }

    // FFTの入力サイズは2のべき乗にする
    size_t fftSize = 1;
    while (fftSize < numSamples) fftSize <<= 1;

    std::vector<std::complex<float>> data(fftSize, 0.0f);
    for (size_t i = 0; i < numSamples; ++i) {
        data[i] = static_cast<float>(samples[i]) / 32768.0f;
    }

    fft(data);

    const float sampleRate = static_cast<float>(audioData.wfex.nSamplesPerSec);
    const size_t outputSize = fftSize / 2;
    result.magnitudes.reserve(outputSize);
    result.frequencies.reserve(outputSize);

    const float frequencyResolution = sampleRate / static_cast<float>(fftSize);

    for (size_t k = 0; k < outputSize; ++k) {
        result.magnitudes.push_back(std::abs(data[k]));
        result.frequencies.push_back(static_cast<float>(k) * frequencyResolution);
    }

    return result;
}

void MyAudioMath::fft(std::vector<std::complex<float>>& a)
{
    const size_t N = a.size();
    if (N <= 1) return;

    // 偶数・奇数に分割
    std::vector<std::complex<float>> even(N / 2), odd(N / 2);
    for (size_t i = 0; i < N / 2; ++i) {
        even[i] = a[i * 2];
        odd[i] = a[i * 2 + 1];
    }
    fft(even);
    fft(odd);

    for (size_t k = 0; k < N / 2; ++k) {
        std::complex<float> t = std::polar<float>(1.0f, -2.0f * float(std::numbers::pi) * k / N) * odd[k];
        a[k] = even[k] + t;
        a[k + N / 2] = even[k] - t;
    }
}