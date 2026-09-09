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

void WaveformDisplay::setGrainPositions (const std::vector<float>& positions)
{
    grainPositions = positions;
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

    auto bounds = getLocalBounds().reduced (6);

    if (hasSource && thumbnail.getTotalLength() > 0.0)
    {
        g.setColour (juce::Colour (0xff3fd6c0));
        thumbnail.drawChannels (g, bounds, 0.0, thumbnail.getTotalLength(), 1.0f);

        // Grãos ativos, desenhados por cima da waveform (posição 0-1 no
        // buffer mapeada para a largura da área da waveform).
        g.setColour (juce::Colour (0xffc084fc).withAlpha (0.85f));
        for (auto pos : grainPositions)
        {
            if (pos < 0.0f)
                continue;

            const auto x = (float) bounds.getX() + pos * (float) bounds.getWidth();
            const auto y = (float) bounds.getCentreY();
            g.fillEllipse (x - 2.5f, y - 2.5f, 5.0f, 5.0f);
        }
    }
    else
    {
        g.setColour (juce::Colour (0xff5a606b));
        g.setFont (12.0f);
        g.drawFittedText ("sem sample - carrega ou arrasta um ficheiro",
                           getLocalBounds(), juce::Justification::centred, 1);
    }
}
