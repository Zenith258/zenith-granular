#pragma once

#include <juce_dsp/juce_dsp.h>
#include <atomic>

struct FilterEngineParams
{
    std::atomic<float>* type      = nullptr; // índice do ComboBox: 0=Low-Pass, 1=High-Pass, 2=Band-Pass
    std::atomic<float>* cutoff    = nullptr; // Hz
    std::atomic<float>* resonance = nullptr; // %
};

/** Filtro multimodo — aplicado ao sinal final, logo após o granulador (ver Signal Flow Diagram). */
class FilterEngine
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void process (juce::AudioBuffer<float>& buffer);
    void connectParameters (FilterEngineParams p) { params = p; }

private:
    juce::dsp::StateVariableTPTFilter<float> filter;
    FilterEngineParams params;
};
