#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay() : thumbnail (512, formatManager, thumbnailCache)
{
    formatManager.registerBasicFormats();
    thumbnail.addChangeListener (this);
}

WaveformDisplay::~WaveformDisplay()
{
    thumbnail.removeChangeListener (this);
}

void WaveformDisplay::setSource (const juce::File& file)
{
    if (file.existsAsFile())
    {
        thumbnail.setSource (new juce::FileInputSource (file));
        hasSource = true;
    }
    else
    {
        thumbnail.clear();
        hasSource = false;
    }
    repaint();
}

void WaveformDisplay::changeListenerCallback (juce::ChangeBroadcaster*)
{
    repaint();
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    g.setColour (juce::Colour (0xff1a1d24));
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 8.0f);

    if (hasSource && thumbnail.getTotalLength() > 0.0)
    {
        g.setColour (juce::Colour (0xff3fd6c0));
        thumbnail.drawChannels (g, getLocalBounds().reduced (6), 0.0, thumbnail.getTotalLength(), 1.0f);
    }
    else
    {
        g.setColour (juce::Colour (0xff5a606b));
        g.setFont (12.0f);
        g.drawFittedText ("sem sample — carrega ou arrasta um ficheiro",
                           getLocalBounds(), juce::Justification::centred, 1);
    }
}
