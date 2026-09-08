#pragma once

#include <juce_dsp/juce_dsp.h>
#include <atomic>

struct DelayEngineParams
{
    std::atomic<float>* timeMs   = nullptr; // ms
    std::atomic<float>* feedback = nullptr; // %
    std::atomic<float>* mix      = nullptr; // %
};

/**
    Delay simples com feedback, tempo em milissegundos. Sincronização ao BPM
    do FL Studio fica para uma fase de refinamento posterior (precisa de ler
    o AudioPlayHead do processor, que ainda não está ligado).
*/
class DelayEngine
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void process (juce::AudioBuffer<float>& buffer);
    void connectParameters (DelayEngineParams p) { params = p; }

private:
    juce::dsp::DelayLine<float> delayLine { 192000 }; // até 4s a 48kHz
    double currentSampleRate = 44100.0;
    DelayEngineParams params;
};
