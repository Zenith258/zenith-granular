#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    void setupRotary (juce::Slider& slider)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
    }
}

ZenithGranularAudioProcessorEditor::ZenithGranularAudioProcessorEditor (ZenithGranularAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    titleLabel.setText ("ZenithGranular - v0.2 (fase 2)", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    loadButton.onClick = [this] { openFileChooser(); };
    addAndMakeVisible (loadButton);

    statusLabel.setText (audioProcessor.hasSampleLoaded() ? "sample carregado" : "sem sample",
                          juce::dontSendNotification);
    statusLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (statusLabel);

    for (auto* s : { &attackSlider, &decaySlider, &sustainSlider, &releaseSlider, &pitchSlider, &fineTuneSlider })
        setupRotary (*s);

    for (auto* l : { &attackLabel, &decayLabel, &sustainLabel, &releaseLabel, &pitchLabel, &fineTuneLabel })
        l->setJustificationType (juce::Justification::centred);

    for (auto* c : { (juce::Component*) &attackSlider, (juce::Component*) &decaySlider,
                      (juce::Component*) &sustainSlider, (juce::Component*) &releaseSlider,
                      (juce::Component*) &pitchSlider, (juce::Component*) &fineTuneSlider,
                      (juce::Component*) &attackLabel, (juce::Component*) &decayLabel,
                      (juce::Component*) &sustainLabel, (juce::Component*) &releaseLabel,
                      (juce::Component*) &pitchLabel, (juce::Component*) &fineTuneLabel })
        addAndMakeVisible (c);

    auto& state = audioProcessor.apvts;
    attackAttachment    = std::make_unique<SliderAttachment> (state, "samplerAttack",    attackSlider);
    decayAttachment     = std::make_unique<SliderAttachment> (state, "samplerDecay",     decaySlider);
    sustainAttachment   = std::make_unique<SliderAttachment> (state, "samplerSustain",   sustainSlider);
    releaseAttachment   = std::make_unique<SliderAttachment> (state, "samplerRelease",   releaseSlider);
    pitchAttachment     = std::make_unique<SliderAttachment> (state, "samplerPitch",     pitchSlider);
    fineTuneAttachment  = std::make_unique<SliderAttachment> (state, "samplerFineTune",  fineTuneSlider);

    setSize (460, 360);
}

ZenithGranularAudioProcessorEditor::~ZenithGranularAudioProcessorEditor() = default;

void ZenithGranularAudioProcessorEditor::openFileChooser()
{
    fileChooser = std::make_unique<juce::FileChooser> ("Escolhe um ficheiro .wav",
                                                         juce::File(),
                                                         "*.wav");

    const auto flags = juce::FileBrowserComponent::openMode
                      | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync (flags, [this] (const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file.existsAsFile())
        {
            audioProcessor.loadSample (file);
            statusLabel.setText ("sample: " + file.getFileName(), juce::dontSendNotification);
        }
    });
}

void ZenithGranularAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff12141a));
}

void ZenithGranularAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (10);

    titleLabel.setBounds (area.removeFromTop (24));
    loadButton.setBounds (area.removeFromTop (30).reduced (60, 0));
    statusLabel.setBounds (area.removeFromTop (20));

    area.removeFromTop (10);

    auto knobRow = area.removeFromTop (110);
    const int knobWidth = knobRow.getWidth() / 6;

    for (auto& pair : { std::pair<juce::Slider*, juce::Label*> (&attackSlider, &attackLabel),
                         std::pair<juce::Slider*, juce::Label*> (&decaySlider, &decayLabel),
                         std::pair<juce::Slider*, juce::Label*> (&sustainSlider, &sustainLabel),
                         std::pair<juce::Slider*, juce::Label*> (&releaseSlider, &releaseLabel),
                         std::pair<juce::Slider*, juce::Label*> (&pitchSlider, &pitchLabel),
                         std::pair<juce::Slider*, juce::Label*> (&fineTuneSlider, &fineTuneLabel) })
    {
        auto column = knobRow.removeFromLeft (knobWidth);
        pair.second->setBounds (column.removeFromBottom (18));
        pair.first->setBounds (column);
    }
}
