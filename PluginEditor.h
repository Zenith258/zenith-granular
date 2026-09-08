#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

/**
    FASE 4 — Editor com os controlos de FX (Filtro, Saturação, Delay, Reverb)
    somados aos do sampler/granular. Continua a ser um layout funcional, não
    o design final (isso é a FASE 8).
*/
class ZenithGranularAudioProcessorEditor : public juce::AudioProcessorEditor,
                                            public juce::FileDragAndDropTarget
{
public:
    explicit ZenithGranularAudioProcessorEditor (ZenithGranularAudioProcessor&);
    ~ZenithGranularAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

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

    juce::ComboBox filterTypeBox, satModeBox;
    juce::Slider filterCutoffSlider, filterResonanceSlider, satDriveSlider, satMixSlider,
                 delayTimeSlider, delayFeedbackSlider, delayMixSlider,
                 reverbSizeSlider, reverbDampingSlider, reverbMixSlider;
    juce::Label filterTypeLabel { {}, "Filter Type" }, filterCutoffLabel { {}, "Cutoff" },
                filterResonanceLabel { {}, "Resonance" }, satModeLabel { {}, "Sat Mode" },
                satDriveLabel { {}, "Drive" }, satMixLabel { {}, "Sat Mix" },
                delayTimeLabel { {}, "Delay Time" }, delayFeedbackLabel { {}, "Feedback" },
                delayMixLabel { {}, "Delay Mix" }, reverbSizeLabel { {}, "Reverb Size" },
                reverbDampingLabel { {}, "Damping" }, reverbMixLabel { {}, "Reverb Mix" };

    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> attackAttachment, decayAttachment, sustainAttachment,
                                       releaseAttachment, pitchAttachment, fineTuneAttachment,
                                       grainSizeAttachment, grainDensityAttachment, grainPositionAttachment,
                                       grainPositionRandomAttachment, grainPitchAttachment,
                                       grainPitchRandomAttachment, grainPanAttachment, granularMixAttachment,
                                       filterCutoffAttachment, filterResonanceAttachment,
                                       satDriveAttachment, satMixAttachment,
                                       delayTimeAttachment, delayFeedbackAttachment, delayMixAttachment,
                                       reverbSizeAttachment, reverbDampingAttachment, reverbMixAttachment;
    std::unique_ptr<ComboBoxAttachment> filterTypeAttachment, satModeAttachment;

    std::unique_ptr<juce::FileChooser> fileChooser;
    bool isDraggingFileOver = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZenithGranularAudioProcessorEditor)
};
