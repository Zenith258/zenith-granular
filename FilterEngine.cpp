#include "FilterEngine.h"

void FilterEngine::prepare (const juce::dsp::ProcessSpec& spec)
{
    filter.prepare (spec);
    filter.setCutoffFrequency (20000.0f);
    filter.setResonance (0.7f);
}

void FilterEngine::process (juce::AudioBuffer<float>& buffer)
{
    const auto typeIndex = params.type != nullptr ? (int) params.type->load() : 0;
    switch (typeIndex)
    {
        case 1:  filter.setType (juce::dsp::StateVariableTPTFilterType::highpass); break;
        case 2:  filter.setType (juce::dsp::StateVariableTPTFilterType::bandpass); break;
        default: filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);  break;
    }

    const auto cutoff = params.cutoff != nullptr
                           ? juce::jlimit (20.0f, 20000.0f, params.cutoff->load())
                           : 20000.0f;
    const auto resonance = params.resonance != nullptr
                              ? juce::jmap (juce::jlimit (0.0f, 100.0f, params.resonance->load()), 0.0f, 100.0f, 0.7f, 10.0f)
                              : 0.7f;

    filter.setCutoffFrequency (cutoff);
    filter.setResonance (resonance);

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    filter.process (context);
}
