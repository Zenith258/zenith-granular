#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

/**
    Guarda e carrega o estado completo da APVTS (todos os knobs) em ficheiros
    .zgpreset. Presets de fábrica são protegidos contra remoção (registados
    num manifesto). Cada preset pode ter uma classificação de 0-3 estrelas.
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

    bool isFactoryPreset (const juce::String& name) const;

    int getRating (const juce::String& name) const;          // 0-3
    void setRating (const juce::String& name, int stars);    // 0-3
    juce::StringArray getFavoritePresets() const;             // rating > 0

private:
    void ensureFactoryPresets();
    static juce::File getRatingsFile();
    static juce::File getFactoryManifestFile();

    juce::AudioProcessorValueTreeState& state;
};
