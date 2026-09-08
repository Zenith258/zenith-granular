#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ZenithLookAndFeel.h"
#include "WaveformDisplay.h"

/**
    FASE 4b — Interface organizada em abas (Sampler / Granular / FX), em vez
    de uma coluna única gigante. Continua a ser um layout funcional, não o
    design definitivo (isso é a FASE 7).
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
    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    /** Uma linha de knob+label, para não repetir o mesmo código em cada aba. */
    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<SliderAttachment> attachment;

        void setup (juce::Component& parent, juce::AudioProcessorValueTreeState& state,
                    const juce::String& paramID, const juce::String& labelText);
        void layout (juce::Rectangle<int> bounds);
    };

    struct SamplerPanel : public juce::Component
    {
        explicit SamplerPanel (ZenithGranularAudioProcessor& proc);
        void resized() override;
        Knob attack, decay, sustain, release, pitch, fineTune;
    };

    struct GranularPanel : public juce::Component
    {
        explicit GranularPanel (ZenithGranularAudioProcessor& proc);
        void resized() override;
        Knob grainSize, grainDensity, grainPosition, grainPositionRandom,
             grainPitch, grainPitchRandom, grainPan, granularMix;
    };

    struct FXPanel : public juce::Component
    {
        explicit FXPanel (ZenithGranularAudioProcessor& proc);
        void resized() override;

        juce::ComboBox filterTypeBox, satModeBox;
        juce::Label filterTypeLabel { {}, "Filter Type" }, satModeLabel { {}, "Sat Mode" };
        std::unique_ptr<ComboBoxAttachment> filterTypeAttachment, satModeAttachment;

        Knob filterCutoff, filterResonance, satDrive, satMix,
             delayTime, delayFeedback, delayMix, reverbSize, reverbDamping, reverbMix;
    };

    struct MacroPanel : public juce::Component
    {
        explicit MacroPanel (ZenithGranularAudioProcessor& proc);
        void resized() override;
        Knob space, texture, movement, chaos;
    };

    void openFileChooser();
    void loadFile (const juce::File& file);
    void refreshPresetList();
    void promptSavePreset();

    ZenithGranularAudioProcessor& audioProcessor;
    ZenithLookAndFeel lookAndFeel;

    juce::Label titleLabel;
    juce::ComboBox presetBox;
    juce::TextButton favoriteButton { juce::String::fromUTF8 ("\xE2\x98\x86") }; // estrela vazia
    juce::TextButton savePresetButton { "Save" };
    juce::TextButton deletePresetButton { "Delete" };
    juce::ToggleButton showFavoritesOnlyToggle { "Favoritos" };
    std::unique_ptr<juce::AlertWindow> presetNameWindow;

    juce::TextButton loadButton { "Load Sample..." };
    juce::Label statusLabel;

    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };
    WaveformDisplay waveformDisplay;
    SamplerPanel samplerPanel;
    GranularPanel granularPanel;
    FXPanel fxPanel;
    MacroPanel macroPanel;

    std::unique_ptr<juce::FileChooser> fileChooser;
    bool isDraggingFileOver = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZenithGranularAudioProcessorEditor)
};
