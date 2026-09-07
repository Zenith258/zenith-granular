#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    void setupRotary (juce::Slider& slider)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 18);
    }

    bool isSupportedAudioFile (const juce::String& path)
    {
        static const juce::StringArray extensions { ".wav", ".aiff", ".aif", ".flac",
                                                      ".ogg", ".mp3", ".m4a", ".wma" };
        const auto lower = path.toLowerCase();
        for (auto& ext : extensions)
            if (lower.endsWith (ext))
                return true;
        return false;
    }
}

ZenithGranularAudioProcessorEditor::ZenithGranularAudioProcessorEditor (ZenithGranularAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    titleLabel.setText ("ZenithGranular - v0.3 (fase 3)", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    loadButton.onClick = [this] { openFileChooser(); };
    addAndMakeVisible (loadButton);

    statusLabel.setText (audioProcessor.hasSampleLoaded() ? "sample carregado" : "sem sample (ou arrasta um ficheiro aqui)",
                          juce::dontSendNotification);
    statusLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (statusLabel);

    for (auto* s : { &attackSlider, &decaySlider, &sustainSlider, &releaseSlider, &pitchSlider, &fineTuneSlider,
                      &grainSizeSlider, &grainDensitySlider, &grainPositionSlider, &grainPositionRandomSlider,
                      &grainPitchSlider, &grainPitchRandomSlider, &grainPanSlider, &granularMixSlider })
        setupRotary (*s);

    for (auto* l : { &attackLabel, &decayLabel, &sustainLabel, &releaseLabel, &pitchLabel, &fineTuneLabel,
                      &grainSizeLabel, &grainDensityLabel, &grainPositionLabel, &grainPositionRandomLabel,
                      &grainPitchLabel, &grainPitchRandomLabel, &grainPanLabel, &granularMixLabel })
        l->setJustificationType (juce::Justification::centred);

    for (auto* c : { (juce::Component*) &attackSlider, (juce::Component*) &decaySlider,
                      (juce::Component*) &sustainSlider, (juce::Component*) &releaseSlider,
                      (juce::Component*) &pitchSlider, (juce::Component*) &fineTuneSlider,
                      (juce::Component*) &attackLabel, (juce::Component*) &decayLabel,
                      (juce::Component*) &sustainLabel, (juce::Component*) &releaseLabel,
                      (juce::Component*) &pitchLabel, (juce::Component*) &fineTuneLabel,
                      (juce::Component*) &grainSizeSlider, (juce::Component*) &grainDensitySlider,
                      (juce::Component*) &grainPositionSlider, (juce::Component*) &grainPositionRandomSlider,
                      (juce::Component*) &grainPitchSlider, (juce::Component*) &grainPitchRandomSlider,
                      (juce::Component*) &grainPanSlider, (juce::Component*) &granularMixSlider,
                      (juce::Component*) &grainSizeLabel, (juce::Component*) &grainDensityLabel,
                      (juce::Component*) &grainPositionLabel, (juce::Component*) &grainPositionRandomLabel,
                      (juce::Component*) &grainPitchLabel, (juce::Component*) &grainPitchRandomLabel,
                      (juce::Component*) &grainPanLabel, (juce::Component*) &granularMixLabel })
        addAndMakeVisible (c);

    auto& state = audioProcessor.apvts;
    attackAttachment    = std::make_unique<SliderAttachment> (state, "samplerAttack",    attackSlider);
    decayAttachment     = std::make_unique<SliderAttachment> (state, "samplerDecay",     decaySlider);
    sustainAttachment   = std::make_unique<SliderAttachment> (state, "samplerSustain",   sustainSlider);
    releaseAttachment   = std::make_unique<SliderAttachment> (state, "samplerRelease",   releaseSlider);
    pitchAttachment     = std::make_unique<SliderAttachment> (state, "samplerPitch",     pitchSlider);
    fineTuneAttachment  = std::make_unique<SliderAttachment> (state, "samplerFineTune",  fineTuneSlider);

    grainSizeAttachment           = std::make_unique<SliderAttachment> (state, "grainSize",            grainSizeSlider);
    grainDensityAttachment        = std::make_unique<SliderAttachment> (state, "grainDensity",         grainDensitySlider);
    grainPositionAttachment       = std::make_unique<SliderAttachment> (state, "grainPosition",         grainPositionSlider);
    grainPositionRandomAttachment = std::make_unique<SliderAttachment> (state, "grainPositionRandom",   grainPositionRandomSlider);
    grainPitchAttachment          = std::make_unique<SliderAttachment> (state, "grainPitch",            grainPitchSlider);
    grainPitchRandomAttachment    = std::make_unique<SliderAttachment> (state, "grainPitchRandom",      grainPitchRandomSlider);
    grainPanAttachment            = std::make_unique<SliderAttachment> (state, "grainPan",              grainPanSlider);
    granularMixAttachment         = std::make_unique<SliderAttachment> (state, "granularMix",           granularMixSlider);

    setSize (620, 480);
}

ZenithGranularAudioProcessorEditor::~ZenithGranularAudioProcessorEditor() = default;

void ZenithGranularAudioProcessorEditor::loadFile (const juce::File& file)
{
    audioProcessor.loadSample (file);
    statusLabel.setText ("sample: " + file.getFileName(), juce::dontSendNotification);
}

void ZenithGranularAudioProcessorEditor::openFileChooser()
{
    fileChooser = std::make_unique<juce::FileChooser> ("Escolhe um ficheiro de áudio",
                                                         juce::File(),
                                                         "*.wav;*.aiff;*.flac;*.ogg;*.mp3;*.m4a;*.wma");

    const auto flags = juce::FileBrowserComponent::openMode
                      | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync (flags, [this] (const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file.existsAsFile())
            loadFile (file);
    });
}

bool ZenithGranularAudioProcessorEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    return files.size() > 0 && isSupportedAudioFile (files[0]);
}

void ZenithGranularAudioProcessorEditor::fileDragEnter (const juce::StringArray&, int, int)
{
    isDraggingFileOver = true;
    repaint();
}

void ZenithGranularAudioProcessorEditor::fileDragExit (const juce::StringArray&)
{
    isDraggingFileOver = false;
    repaint();
}

void ZenithGranularAudioProcessorEditor::filesDropped (const juce::StringArray& files, int, int)
{
    isDraggingFileOver = false;
    repaint();

    if (files.size() > 0)
    {
        juce::File file (files[0]);
        if (file.existsAsFile())
            loadFile (file);
    }
}

void ZenithGranularAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff12141a));

    if (isDraggingFileOver)
    {
        g.setColour (juce::Colour (0xff3fd6c0));
        g.drawRect (getLocalBounds().reduced (4), 2);
    }
}

void ZenithGranularAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (10);

    titleLabel.setBounds (area.removeFromTop (24));
    loadButton.setBounds (area.removeFromTop (30).reduced (60, 0));
    statusLabel.setBounds (area.removeFromTop (20));

    area.removeFromTop (8);

    auto layoutRow = [] (juce::Rectangle<int> row, std::initializer_list<std::pair<juce::Slider*, juce::Label*>> items)
    {
        const int w = row.getWidth() / (int) items.size();
        for (auto& pair : items)
        {
            auto column = row.removeFromLeft (w);
            pair.second->setBounds (column.removeFromBottom (18));
            pair.first->setBounds (column);
        }
    };

    layoutRow (area.removeFromTop (100),
        { { &attackSlider, &attackLabel }, { &decaySlider, &decayLabel }, { &sustainSlider, &sustainLabel },
          { &releaseSlider, &releaseLabel }, { &pitchSlider, &pitchLabel }, { &fineTuneSlider, &fineTuneLabel } });

    area.removeFromTop (10);

    layoutRow (area.removeFromTop (100),
        { { &grainSizeSlider, &grainSizeLabel }, { &grainDensitySlider, &grainDensityLabel },
          { &grainPositionSlider, &grainPositionLabel }, { &grainPositionRandomSlider, &grainPositionRandomLabel } });

    area.removeFromTop (10);

    layoutRow (area.removeFromTop (100),
        { { &grainPitchSlider, &grainPitchLabel }, { &grainPitchRandomSlider, &grainPitchRandomLabel },
          { &grainPanSlider, &grainPanLabel }, { &granularMixSlider, &granularMixLabel } });
}
