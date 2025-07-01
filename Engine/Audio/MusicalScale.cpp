#include "MusicalScale.h"
#include <cmath>

float GetFrequencyFormGermanNote(GermanNote note, int octave) {
    float result{};
    result = static_cast<float>(note) * 0.01f;

    int octaveCorrection = octave - 4;
    result *= std::powf(2.0f,static_cast<float>(octaveCorrection));

    return result;
}
