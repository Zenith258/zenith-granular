#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

/**
    FASE 2 — Editor funcional mínimo: um botão para carregar o .wav e knobs
    ligados diretamente à APVTS (usando SliderAttachment — assim ficam
    automatizáveis pelo DAW automaticamente, sem código extra). O layout
    visual definitivo (waveform, grãos) entra na FASE 8.
*/
class ZenithGranularAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ZenithGranularAudioProcessorEditor (ZenithGranularAudioProcessor&);
    ~ZenithGranularAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void openFileChooser();

    ZenithGranularAudioProcessor& audioProcessor;

    juce::Label titleLabel;
    juce::TextButton loadButton { "Load Sample..." };
    juce::Label statusLabel;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider, pitchSlider, fineTuneSlider;
    juce::Label attackLabel { {}, "Attack" }, decayLabel { {}, "Decay" }, sustainLabel { {}, "Sustain" },
                releaseLabel { {}, "Release" }, pitchLabel { {}, "Pitch" }, fineTuneLabel { {}, "Fine Tune" };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> attackAttachment, decayAttachment, sustainAttachment,
                                       releaseAttachment, pitchAttachment, fineTuneAttachment;

    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZenithGranularAudioProcessorEditor)
};
