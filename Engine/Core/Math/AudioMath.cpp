#include "AudioMath.h"

float MyAudioMath::SpeedOfSoundFromTemperature(float celsius) {
    return 331.5f + 0.6f * celsius;
}
