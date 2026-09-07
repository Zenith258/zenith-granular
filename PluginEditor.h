#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

/**
    FASE 3 — Editor com os knobs granulares e suporte a arrastar-e-largar
    (arrasta um ficheiro de áudio para cima da janela do plugin em vez de
    teres de clicar em "Load Sample..."). O layout visual definitivo
    (waveform, grãos animados) entra na FASE 8.
*/
class ZenithGranularAudioProcessorEditor : public juce::AudioProcessorEditor,
                                            public juce::FileDragAndDropTarget
{
public:
    explicit ZenithGranularAudioProcessorEditor (ZenithGranularAudioProcessor&);
    ~ZenithGranularAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // --- juce::FileDragAndDropTarget ---
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void fileDragEnter (const juce::StringArray& files, int x, int y) override;
    void fileDragExit (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

private:
    void openFileChooser();
    void loadFile (const juce::File& file);

    ZenithGranularAudioProcessor& audioProcessor;

    juce::Label titleLabel;
    juce::TextButton loadButton { "Load Sample..." };
    juce::Label statusLabel;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider, pitchSlider, fineTuneSlider;
    juce::Label attackLabel { {}, "Attack" }, decayLabel { {}, "Decay" }, sustainLabel { {}, "Sustain" },
                releaseLabel { {}, "Release" }, pitchLabel { {}, "Pitch" }, fineTuneLabel { {}, "Fine Tune" };

    juce::Slider grainSizeSlider, grainDensitySlider, grainPositionSlider, grainPositionRandomSlider,
                 grainPitchSlider, grainPitchRandomSlider, grainPanSlider, granularMixSlider;
    juce::Label grainSizeLabel { {}, "Grain Size" }, grainDensityLabel { {}, "Density" },
                grainPositionLabel { {}, "Position" }, grainPositionRandomLabel { {}, "Pos. Random" },
                grainPitchLabel { {}, "Grain Pitch" }, grainPitchRandomLabel { {}, "Pitch Random" },
                grainPanLabel { {}, "Pan Spread" }, granularMixLabel { {}, "Granular Mix" };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> attackAttachment, decayAttachment, sustainAttachment,
                                       releaseAttachment, pitchAttachment, fineTuneAttachment,
                                       grainSizeAttachment, grainDensityAttachment, grainPositionAttachment,
                                       grainPositionRandomAttachment, grainPitchAttachment,
                                       grainPitchRandomAttachment, grainPanAttachment, granularMixAttachment;

    std::unique_ptr<juce::FileChooser> fileChooser;
    bool isDraggingFileOver = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZenithGranularAudioProcessorEditor)
};
