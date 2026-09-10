#include "DelayEngine.h"

void DelayEngine::prepare (const juce::dsp::ProcessSpec& spec)
{
    currentSampleRate = spec.sampleRate;

    // 2.5s de margem acima do máximo do parâmetro (2s) — calculado a partir
    // do sample rate real do projeto, em vez de um número fixo que só
    // funcionava até 48kHz.
    delayLine.setMaximumDelayInSamples ((int) (2.5 * spec.sampleRate));
    delayLine.prepare (spec);
    delayLine.reset();
}

void DelayEngine::process (juce::AudioBuffer<float>& buffer)
{
    const auto mix = params.mix != nullptr ? juce::jlimit (0.0f, 1.0f, params.mix->load() / 100.0f) : 0.0f;
    if (mix <= 0.0001f)
        return;

    const auto timeMs   = params.timeMs   != nullptr ? juce::jlimit (1.0f, 2000.0f, params.timeMs->load()) : 300.0f;
    const auto feedback = params.feedback != nullptr ? juce::jlimit (0.0f, 0.95f, params.feedback->load() / 100.0f) : 0.3f;

    delayLine.setDelay ((float) (timeMs / 1000.0 * currentSampleRate));

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto dry = data[i];
            const auto delayed = delayLine.popSample (ch);
            delayLine.pushSample (ch, dry + delayed * feedback);
            data[i] = dry * (1.0f - mix) + delayed * mix;
        }
    }
}
