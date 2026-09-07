#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

/**
    FASE 1 — Editor mínimo. Só prova que a UI abre e está ligada ao processor.
    O layout real (waveform, grãos, knobs) entra na FASE 8, depois de todo o
    DSP estar a funcionar sem UI — regra da arquitetura: DSP não depende da UI.
*/
class ZenithGranularAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ZenithGranularAudioProcessorEditor (ZenithGranularAudioProcessor&);
    ~ZenithGranularAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Referência ao processor — a UI lê o estado dele, nunca faz DSP aqui.
    ZenithGranularAudioProcessor& audioProcessor;

    juce::Label titleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZenithGranularAudioProcessorEditor)
};
