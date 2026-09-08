#include "SaturationEngine.h"
#include <cmath>

void SaturationEngine::process (juce::AudioBuffer<float>& buffer)
{
    const auto mix = params.mix != nullptr ? juce::jlimit (0.0f, 1.0f, params.mix->load() / 100.0f) : 0.0f;
    if (mix <= 0.0001f)
        return;

    const auto drive = params.drive != nullptr ? juce::jlimit (0.0f, 1.0f, params.drive->load() / 100.0f) : 0.0f;
    const auto mode  = params.mode  != nullptr ? (int) params.mode->load() : 0;
    const auto driveAmount = 1.0f + drive * 9.0f; // 1x a 10x

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto dry = data[i];
            const auto driven = dry * driveAmount;
            float wet;

            switch (mode)
            {
                case 1: // "tape" — tangente hiperbólica suave
                    wet = std::tanh (driven * 0.7f);
                    break;
                case 2: // "tube" — assimétrico, mais harmônicos pares
                    wet = driven >= 0.0f ? std::tanh (driven) : std::tanh (driven * 0.6f);
                    break;
                default: // soft clip polinomial
                {
                    const auto clipped = juce::jlimit (-1.0f, 1.0f, driven);
                    wet = clipped - (clipped * clipped * clipped) / 3.0f;
                    break;
                }
            }

            data[i] = dry * (1.0f - mix) + wet * mix;
        }
    }
}
