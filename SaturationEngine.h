#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>

struct SaturationEngineParams
{
    std::atomic<float>* mode  = nullptr; // índice do ComboBox: 0=Soft Clip, 1=Tape, 2=Tube
    std::atomic<float>* drive = nullptr; // %
    std::atomic<float>* mix   = nullptr; // %
};

/** Módulo de saturação/warmth — aplicado depois do filtro, antes do delay. */
class SaturationEngine
{
public:
    void process (juce::AudioBuffer<float>& buffer);
    void connectParameters (SaturationEngineParams p) { params = p; }

private:
    SaturationEngineParams params;
};
