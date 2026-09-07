#include "PluginProcessor.h"
#include "PluginEditor.h"

ZenithGranularAudioProcessorEditor::ZenithGranularAudioProcessorEditor (ZenithGranularAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    titleLabel.setText ("ZenithGranular - v0.1 (fase 1)", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    setSize (400, 300);
}

ZenithGranularAudioProcessorEditor::~ZenithGranularAudioProcessorEditor() = default;

void ZenithGranularAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff12141a));
}

void ZenithGranularAudioProcessorEditor::resized()
{
    titleLabel.setBounds (getLocalBounds());
}
