#pragma once

namespace MyAudioMath {
	/// <summary>
	///気温から空気中でのおおまかな音速を求めます 
	/// </summary>
	/// <param name="celsius">摂氏</param>
	/// <returns></returns>
	float SpeedOfSoundFromTemperature(float celsius);
};