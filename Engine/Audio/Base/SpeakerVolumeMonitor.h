#pragma once
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <thread>

class SpeakerVolumeMonitor final
{
public:
    SpeakerVolumeMonitor();
    ~SpeakerVolumeMonitor();

    bool Initialize();
    void Start();
    void Stop();
    void Finalize();

    float GetRMSLevel() const;
    float GetPeakLevel() const;
    bool IsCapturing() const;

private:
    
};