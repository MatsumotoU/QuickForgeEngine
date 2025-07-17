#include "SpeakerVolumeMonitor.h"

SpeakerVolumeMonitor::SpeakerVolumeMonitor() {
}

SpeakerVolumeMonitor::~SpeakerVolumeMonitor() {
}

bool SpeakerVolumeMonitor::Initialize() {
	return false;
}

void SpeakerVolumeMonitor::Start() {
}

void SpeakerVolumeMonitor::Stop() {
}

void SpeakerVolumeMonitor::Finalize() {
}

float SpeakerVolumeMonitor::GetRMSLevel() const {
	return 0.0f;
}

float SpeakerVolumeMonitor::GetPeakLevel() const {
	return 0.0f;
}

bool SpeakerVolumeMonitor::IsCapturing() const {
	return false;
}
