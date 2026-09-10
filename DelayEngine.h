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
    Delay simples com feedback, tempo em milissegundos. O buffer interno é
    dimensionado em prepare() com base no sample rate REAL do projeto — um
    tamanho fixo (calculado só para 44.1/48kHz) cortaria o delay em projetos
    a 96kHz ou 192kHz. Sincronização ao BPM do FL Studio fica para uma fase
    de refinamento posterior.
*/
class DelayEngine
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void process (juce::AudioBuffer<float>& buffer);
    void connectParameters (DelayEngineParams p) { params = p; }

private:
    juce::dsp::DelayLine<float> delayLine;
    double currentSampleRate = 44100.0;
    DelayEngineParams params;
};
