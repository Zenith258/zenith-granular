#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>

/**
    Mostra a waveform real do ficheiro de áudio carregado, usando
    juce::AudioThumbnail (o mesmo mecanismo que o próprio JUCE usa em apps
    de áudio para desenhar formas de onda de forma eficiente).
    A animação dos grãos em cima disto fica para uma fase seguinte.
*/
class WaveformDisplay : public juce::Component,
                         private juce::ChangeListener
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override;

    void setSource (const juce::File& file);
    void setGrainPositions (const std::vector<float>& positions);
    void paint (juce::Graphics& g) override;

private:
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache { 4 };
    juce::AudioThumbnail thumbnail;
    bool hasSource = false;
    std::vector<float> grainPositions;
};
