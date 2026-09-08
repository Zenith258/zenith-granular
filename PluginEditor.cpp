#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
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

// ---------------------------------------------------------------- Knob

void ZenithGranularAudioProcessorEditor::Knob::setup (juce::Component& parent,
                                                        juce::AudioProcessorValueTreeState& state,
                                                        const juce::String& paramID,
                                                        const juce::String& labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 18);
    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);

    parent.addAndMakeVisible (slider);
    parent.addAndMakeVisible (label);

    attachment = std::make_unique<SliderAttachment> (state, paramID, slider);
}

void ZenithGranularAudioProcessorEditor::Knob::layout (juce::Rectangle<int> bounds)
{
    label.setBounds (bounds.removeFromBottom (18));
    slider.setBounds (bounds.reduced (4, 0));
}

// ---------------------------------------------------------------- SamplerPanel

ZenithGranularAudioProcessorEditor::SamplerPanel::SamplerPanel (ZenithGranularAudioProcessor& proc)
{
    attack.setup   (*this, proc.apvts, "samplerAttack",   "Attack");
    decay.setup    (*this, proc.apvts, "samplerDecay",    "Decay");
    sustain.setup  (*this, proc.apvts, "samplerSustain",  "Sustain");
    release.setup  (*this, proc.apvts, "samplerRelease",  "Release");
    pitch.setup    (*this, proc.apvts, "samplerPitch",    "Pitch");
    fineTune.setup (*this, proc.apvts, "samplerFineTune", "Fine Tune");
}

void ZenithGranularAudioProcessorEditor::SamplerPanel::resized()
{
    auto area = getLocalBounds().reduced (10);
    const int w = area.getWidth() / 6;
    for (auto* k : { &attack, &decay, &sustain, &release, &pitch, &fineTune })
        k->layout (area.removeFromLeft (w));
}

// ---------------------------------------------------------------- GranularPanel

ZenithGranularAudioProcessorEditor::GranularPanel::GranularPanel (ZenithGranularAudioProcessor& proc)
{
    grainSize.setup           (*this, proc.apvts, "grainSize",           "Grain Size");
    grainDensity.setup        (*this, proc.apvts, "grainDensity",        "Density");
    grainPosition.setup       (*this, proc.apvts, "grainPosition",       "Position");
    grainPositionRandom.setup (*this, proc.apvts, "grainPositionRandom", "Pos. Random");
    grainPitch.setup          (*this, proc.apvts, "grainPitch",          "Grain Pitch");
    grainPitchRandom.setup    (*this, proc.apvts, "grainPitchRandom",    "Pitch Random");
    grainPan.setup            (*this, proc.apvts, "grainPan",            "Pan Spread");
    granularMix.setup         (*this, proc.apvts, "granularMix",         "Granular Mix");
}

void ZenithGranularAudioProcessorEditor::GranularPanel::resized()
{
    auto area = getLocalBounds().reduced (10);
    auto topRow = area.removeFromTop (area.getHeight() / 2);
    const int w = topRow.getWidth() / 4;
    for (auto* k : { &grainSize, &grainDensity, &grainPosition, &grainPositionRandom })
        k->layout (topRow.removeFromLeft (w));
    for (auto* k : { &grainPitch, &grainPitchRandom, &grainPan, &granularMix })
        k->layout (area.removeFromLeft (w));
}

// ---------------------------------------------------------------- FXPanel

ZenithGranularAudioProcessorEditor::FXPanel::FXPanel (ZenithGranularAudioProcessor& proc)
{
    filterTypeBox.addItemList ({ "Low-Pass", "High-Pass", "Band-Pass" }, 1);
    satModeBox.addItemList ({ "Soft Clip", "Tape", "Tube" }, 1);
    filterTypeLabel.setJustificationType (juce::Justification::centred);
    satModeLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (filterTypeBox);
    addAndMakeVisible (satModeBox);
    addAndMakeVisible (filterTypeLabel);
    addAndMakeVisible (satModeLabel);
    filterTypeAttachment = std::make_unique<ComboBoxAttachment> (proc.apvts, "filterType", filterTypeBox);
    satModeAttachment    = std::make_unique<ComboBoxAttachment> (proc.apvts, "satMode", satModeBox);

    filterCutoff.setup    (*this, proc.apvts, "filterCutoff",    "Cutoff");
    filterResonance.setup (*this, proc.apvts, "filterResonance", "Resonance");
    satDrive.setup        (*this, proc.apvts, "satDrive",        "Drive");
    satMix.setup          (*this, proc.apvts, "satMix",          "Sat Mix");
    delayTime.setup       (*this, proc.apvts, "delayTime",       "Delay Time");
    delayFeedback.setup   (*this, proc.apvts, "delayFeedback",   "Feedback");
    delayMix.setup        (*this, proc.apvts, "delayMix",        "Delay Mix");
    reverbSize.setup      (*this, proc.apvts, "reverbSize",      "Reverb Size");
    reverbDamping.setup   (*this, proc.apvts, "reverbDamping",   "Damping");
    reverbMix.setup       (*this, proc.apvts, "reverbMix",       "Reverb Mix");
}

void ZenithGranularAudioProcessorEditor::FXPanel::resized()
{
    auto area = getLocalBounds().reduced (10);

    auto row1 = area.removeFromTop (area.getHeight() / 3);
    const int w1 = row1.getWidth() / 4;
    {
        auto col = row1.removeFromLeft (w1);
        filterTypeLabel.setBounds (col.removeFromBottom (18));
        filterTypeBox.setBounds (col.reduced (4, 20));
    }
    filterCutoff.layout (row1.removeFromLeft (w1));
    filterResonance.layout (row1.removeFromLeft (w1));
    {
        auto col = row1.removeFromLeft (w1);
        satModeLabel.setBounds (col.removeFromBottom (18));
        satModeBox.setBounds (col.reduced (4, 20));
    }

    auto row2 = area.removeFromTop (area.getHeight() / 2);
    const int w2 = row2.getWidth() / 4;
    for (auto* k : { &satDrive, &satMix, &delayTime, &delayFeedback })
        k->layout (row2.removeFromLeft (w2));

    const int w3 = area.getWidth() / 4;
    for (auto* k : { &delayMix, &reverbSize, &reverbDamping, &reverbMix })
        k->layout (area.removeFromLeft (w3));
}

// ---------------------------------------------------------------- Editor

ZenithGranularAudioProcessorEditor::ZenithGranularAudioProcessorEditor (ZenithGranularAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      samplerPanel (p), granularPanel (p), fxPanel (p)
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

    tabs.addTab ("Sampler",  juce::Colour (0xff12141a), &samplerPanel,  false);
    tabs.addTab ("Granular", juce::Colour (0xff12141a), &granularPanel, false);
    tabs.addTab ("FX",       juce::Colour (0xff12141a), &fxPanel,       false);
    addAndMakeVisible (tabs);

    setSize (480, 400);
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
    tabs.setBounds (area);
}
