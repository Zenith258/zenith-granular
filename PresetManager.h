#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

/**
    Guarda e carrega o estado completo da APVTS (todos os knobs) em ficheiros
    .zgpreset dentro da pasta de dados do utilizador — não depende do projeto
    do DAW, os presets ficam disponíveis em qualquer música.
*/
class PresetManager
{
public:
    explicit PresetManager (juce::AudioProcessorValueTreeState& stateToUse);

    static juce::File getPresetsFolder();
    juce::StringArray getAllPresets() const;
    void savePreset (const juce::String& name);
    void loadPreset (const juce::String& name);
    void deletePreset (const juce::String& name);

private:
    void ensureFactoryPresets();

    juce::AudioProcessorValueTreeState& state;
};
