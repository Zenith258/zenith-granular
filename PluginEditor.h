#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ZenithLookAndFeel.h"
#include "WaveformDisplay.h"
#include <map>
#include <vector>

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
    /** 3 estrelas clicáveis, como a classificação de presets do Zenology. */
    struct StarRating : public juce::Component
    {
        std::function<void (int)> onRatingChanged;
        int rating = 0;

        void paint (juce::Graphics& g) override
        {
            const auto w = getWidth() / 3.0f;
            for (int i = 0; i < 3; ++i)
            {
                const auto filled = i < rating;
                g.setColour (filled ? juce::Colour (0xffffd54f) : juce::Colour (0xff5a606b));
                g.setFont (15.0f);
                g.drawFittedText (juce::String::fromUTF8 ("\xE2\x98\x85"),
                                   juce::Rectangle<int> ((int) (i * w), 0, (int) w, getHeight()),
                                   juce::Justification::centred, 1);
            }
        }

        void mouseUp (const juce::MouseEvent& e) override
        {
            const auto w = getWidth() / 3.0f;
            const auto clicked = juce::jlimit (1, 3, (int) (e.position.x / w) + 1);
            rating = (rating == clicked) ? 0 : clicked; // clicar na mesma estrela limpa
            repaint();
            if (onRatingChanged != nullptr)
                onRatingChanged (rating);
        }
    };

    juce::ComboBox presetBox;
    juce::TextButton categoryMenuButton { "Presets \xE2\x96\xBE" };
    StarRating starRating;
    juce::TextButton favoritesMenuButton { juce::String::fromUTF8 ("\xE2\x98\x85") + " Favoritos" };
    juce::TextButton savePresetButton { "Save" };
    juce::TextButton deletePresetButton { "Delete" };
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
