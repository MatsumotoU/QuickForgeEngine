#pragma once
#include <vector>
#include <complex>
#include <cmath>
#include <stdexcept>
#include <numbers>

namespace QFE {
	struct AudioData;
	struct Spectrum;

	namespace MyAudioMath {
		/// <summary>
		///気温から空気中でのおおまかな音速を求めます 
		/// </summary>
		/// <param name="celsius">摂氏</param>
		/// <returns></returns>
		float SpeedOfSoundFromTemperature(float celsius);

		Spectrum CreateSpectrumFromAudioData(const AudioData& audioData);

		void fft(std::vector<std::complex<float>>& a);
	};
}