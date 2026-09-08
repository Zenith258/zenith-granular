#pragma once

#include <juce_dsp/juce_dsp.h>
#include <atomic>

struct ReverbEngineParams
{
    std::atomic<float>* size    = nullptr; // %
    std::atomic<float>* damping = nullptr; // %
    std::atomic<float>* mix     = nullptr; // %
};

/** Reverb algorítmico — último estágio da cadeia, antes da saída. */
class ReverbEngine
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void process (juce::AudioBuffer<float>& buffer);
    void connectParameters (ReverbEngineParams p) { params = p; }

private:
    juce::dsp::Reverb reverb;
    ReverbEngineParams params;
};
