#include "editor/include/UI/View/InputDebugView.h"
#include "engine/include/input/InputInterface.h"
#include "Engine/include/assets/AudioSource/Data/AudioData.h"
#include <ksmedia.h> // オーディオフォーマットGUIDのために追加

InputDebugView::InputDebugView()
{
	isActive_ = false;
	name_ = "Input Debug View";
}
void InputDebugView::Initialize()
{
	isActive_ = false;
}

void InputDebugView::Update()
{
}

void InputDebugView::Draw()
{
    if (!isActive_) { return; }

    if (ImGui::CollapsingHeader("Input Debug View", ImGuiTreeNodeFlags_DefaultOpen)) {

        InputInterface* input = InputInterface::GetInstance();

        ImGui::Text("Mic State:");
        if (input->GetMicrophoneDevice().IsCapturing()) {
            if (ImGui::Button("Stop Capturing")) {
                input->GetMicrophoneDevice().StopCapture();
            }
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Capturing");
        }
        else {
            if (ImGui::Button("Start Capturing")) {
                input->GetMicrophoneDevice().StartCapture();
            }
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not Capturing");
        }

        AudioData audio = input->GetMicrophoneDevice().GetAudioData();
		ImGui::Text("FormatTag: %d, BitsPerSample: %d", audio.wfxEx.Format.wFormatTag, audio.wfxEx.Format.wBitsPerSample);
        if (audio.buffer.data() && audio.buffer.size() > 0) {
            int sampleCount = static_cast<int>(audio.buffer.size()) / (audio.wfxEx.Format.wBitsPerSample / 8);
            std::vector<float> samples(sampleCount);

			if (audio.wfxEx.Format.wBitsPerSample == 8) {
				uint8_t* byteBuffer = static_cast<uint8_t*>(audio.buffer.data());
				for (int i = 0; i < sampleCount; ++i) {
					samples[i] = static_cast<float>(byteBuffer[i]);
				}
				ImGui::PlotLines("Audio Waveform", samples.data(), static_cast<int>(sampleCount), 0, nullptr, 0.0f, 255.0f, ImVec2(0, 80));
			}
            else if (audio.wfxEx.Format.wBitsPerSample == 16) {
                int16_t* shortBuffer = reinterpret_cast<int16_t*>(audio.buffer.data());
                for (int i = 0; i < sampleCount; ++i) {
                    samples[i] = static_cast<float>(shortBuffer[i]);
                }
                ImGui::PlotLines("Audio Waveform", samples.data(), static_cast<int>(sampleCount), 0, nullptr, -32768.0f, 32767.0f, ImVec2(0, 80));
            }
			else if (audio.wfxEx.Format.wBitsPerSample == 32) {
				bool isFloat = false;
				if (audio.wfxEx.Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
					isFloat = true;
				}
				else if (audio.wfxEx.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
					if (IsEqualGUID(audio.wfxEx.SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
						isFloat = true;
					}
				}

				if (isFloat) {
					// 32bit float の場合
					float* floatBuffer = reinterpret_cast<float*>(audio.buffer.data());
					// この場合、samplesへのコピーは不要だが、PlotLinesがfloatの配列を要求するため、そのまま使う
					ImGui::PlotLines("Audio Waveform (float)", floatBuffer, static_cast<int>(sampleCount), 0, nullptr, -1.0f, 1.0f, ImVec2(0, 80));
				}
				else {
					// 32bit int の場合
					int32_t* intBuffer = reinterpret_cast<int32_t*>(audio.buffer.data());
					for (int i = 0; i < sampleCount; ++i) {
						samples[i] = static_cast<float>(intBuffer[i]);
					}
					ImGui::PlotLines("Audio Waveform (int32)", samples.data(), static_cast<int>(sampleCount), 0, nullptr, -2147483648.0f, 2147483647.0f, ImVec2(0, 80));
				}
			}
        }
        else {
            ImGui::Text("No audio data available.");
        }


    }
}

void InputDebugView::Run()
{
}
