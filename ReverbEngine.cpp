#include "ReverbEngine.h"

void ReverbEngine::prepare (const juce::dsp::ProcessSpec& spec)
{
    reverb.prepare (spec);
    reverb.reset();
}

void ReverbEngine::process (juce::AudioBuffer<float>& buffer)
{
    const auto mix = params.mix != nullptr ? juce::jlimit (0.0f, 1.0f, params.mix->load() / 100.0f) : 0.0f;

    juce::dsp::Reverb::Parameters p;
    p.roomSize = params.size    != nullptr ? juce::jlimit (0.0f, 1.0f, params.size->load()    / 100.0f) : 0.5f;
    p.damping  = params.damping != nullptr ? juce::jlimit (0.0f, 1.0f, params.damping->load() / 100.0f) : 0.5f;
    p.wetLevel = mix;
    p.dryLevel = 1.0f - mix;
    p.width    = 1.0f;
    reverb.setParameters (p);

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    reverb.process (context);
}
