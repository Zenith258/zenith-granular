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
    titleLabel.setText ("ZenithGranular - v0.4 (fase 4)", juce::dontSendNotification);
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
                      &grainPitchSlider, &grainPitchRandomSlider, &grainPanSlider, &granularMixSlider,
                      &filterCutoffSlider, &filterResonanceSlider, &satDriveSlider, &satMixSlider,
                      &delayTimeSlider, &delayFeedbackSlider, &delayMixSlider,
                      &reverbSizeSlider, &reverbDampingSlider, &reverbMixSlider })
        setupRotary (*s);

    for (auto* l : { &attackLabel, &decayLabel, &sustainLabel, &releaseLabel, &pitchLabel, &fineTuneLabel,
                      &grainSizeLabel, &grainDensityLabel, &grainPositionLabel, &grainPositionRandomLabel,
                      &grainPitchLabel, &grainPitchRandomLabel, &grainPanLabel, &granularMixLabel,
                      &filterTypeLabel, &filterCutoffLabel, &filterResonanceLabel, &satModeLabel,
                      &satDriveLabel, &satMixLabel, &delayTimeLabel, &delayFeedbackLabel, &delayMixLabel,
                      &reverbSizeLabel, &reverbDampingLabel, &reverbMixLabel })
        l->setJustificationType (juce::Justification::centred);

    filterTypeBox.addItemList ({ "Low-Pass", "High-Pass", "Band-Pass" }, 1);
    satModeBox.addItemList ({ "Soft Clip", "Tape", "Tube" }, 1);

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
                      (juce::Component*) &grainPanLabel, (juce::Component*) &granularMixLabel,
                      (juce::Component*) &filterTypeBox, (juce::Component*) &filterCutoffSlider,
                      (juce::Component*) &filterResonanceSlider, (juce::Component*) &satModeBox,
                      (juce::Component*) &satDriveSlider, (juce::Component*) &satMixSlider,
                      (juce::Component*) &delayTimeSlider, (juce::Component*) &delayFeedbackSlider,
                      (juce::Component*) &delayMixSlider, (juce::Component*) &reverbSizeSlider,
                      (juce::Component*) &reverbDampingSlider, (juce::Component*) &reverbMixSlider,
                      (juce::Component*) &filterTypeLabel, (juce::Component*) &filterCutoffLabel,
                      (juce::Component*) &filterResonanceLabel, (juce::Component*) &satModeLabel,
                      (juce::Component*) &satDriveLabel, (juce::Component*) &satMixLabel,
                      (juce::Component*) &delayTimeLabel, (juce::Component*) &delayFeedbackLabel,
                      (juce::Component*) &delayMixLabel, (juce::Component*) &reverbSizeLabel,
                      (juce::Component*) &reverbDampingLabel, (juce::Component*) &reverbMixLabel })
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

    filterTypeAttachment      = std::make_unique<ComboBoxAttachment> (state, "filterType", filterTypeBox);
    filterCutoffAttachment    = std::make_unique<SliderAttachment>   (state, "filterCutoff", filterCutoffSlider);
    filterResonanceAttachment = std::make_unique<SliderAttachment>   (state, "filterResonance", filterResonanceSlider);

    satModeAttachment  = std::make_unique<ComboBoxAttachment> (state, "satMode", satModeBox);
    satDriveAttachment = std::make_unique<SliderAttachment>   (state, "satDrive", satDriveSlider);
    satMixAttachment   = std::make_unique<SliderAttachment>   (state, "satMix", satMixSlider);

    delayTimeAttachment     = std::make_unique<SliderAttachment> (state, "delayTime", delayTimeSlider);
    delayFeedbackAttachment = std::make_unique<SliderAttachment> (state, "delayFeedback", delayFeedbackSlider);
    delayMixAttachment      = std::make_unique<SliderAttachment> (state, "delayMix", delayMixSlider);

    reverbSizeAttachment    = std::make_unique<SliderAttachment> (state, "reverbSize", reverbSizeSlider);
    reverbDampingAttachment = std::make_unique<SliderAttachment> (state, "reverbDamping", reverbDampingSlider);
    reverbMixAttachment     = std::make_unique<SliderAttachment> (state, "reverbMix", reverbMixSlider);

    setSize (640, 680);
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

    auto layoutRow = [] (juce::Rectangle<int> row, std::initializer_list<std::pair<juce::Component*, juce::Label*>> items)
    {
        const int w = row.getWidth() / (int) items.size();
        for (auto& pair : items)
        {
            auto column = row.removeFromLeft (w);
            pair.second->setBounds (column.removeFromBottom (18));
            pair.first->setBounds (column.reduced (4, 0));
        }
    };

    layoutRow (area.removeFromTop (90),
        { { &attackSlider, &attackLabel }, { &decaySlider, &decayLabel }, { &sustainSlider, &sustainLabel },
          { &releaseSlider, &releaseLabel }, { &pitchSlider, &pitchLabel }, { &fineTuneSlider, &fineTuneLabel } });

    area.removeFromTop (8);

    layoutRow (area.removeFromTop (90),
        { { &grainSizeSlider, &grainSizeLabel }, { &grainDensitySlider, &grainDensityLabel },
          { &grainPositionSlider, &grainPositionLabel }, { &grainPositionRandomSlider, &grainPositionRandomLabel } });

    area.removeFromTop (8);

    layoutRow (area.removeFromTop (90),
        { { &grainPitchSlider, &grainPitchLabel }, { &grainPitchRandomSlider, &grainPitchRandomLabel },
          { &grainPanSlider, &grainPanLabel }, { &granularMixSlider, &granularMixLabel } });

    area.removeFromTop (8);

    layoutRow (area.removeFromTop (90),
        { { &filterTypeBox, &filterTypeLabel }, { &filterCutoffSlider, &filterCutoffLabel },
          { &filterResonanceSlider, &filterResonanceLabel }, { &satModeBox, &satModeLabel } });

    area.removeFromTop (8);

    layoutRow (area.removeFromTop (90),
        { { &satDriveSlider, &satDriveLabel }, { &satMixSlider, &satMixLabel },
          { &delayTimeSlider, &delayTimeLabel }, { &delayFeedbackSlider, &delayFeedbackLabel } });

    area.removeFromTop (8);

    layoutRow (area.removeFromTop (90),
        { { &delayMixSlider, &delayMixLabel }, { &reverbSizeSlider, &reverbSizeLabel },
          { &reverbDampingSlider, &reverbDampingLabel }, { &reverbMixSlider, &reverbMixLabel } });
}
