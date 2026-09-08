#include "PresetManager.h"
#include <vector>
#include <utility>

namespace
{
    /** Preset de fábrica: só valores de ADSR/Granular/FX — sem samplePath,
        para se aplicar a qualquer sample que o utilizador já tenha carregado. */
    void writeFactoryPreset (juce::AudioProcessorValueTreeState& state, const juce::String& name,
                              const std::vector<std::pair<juce::String, float>>& values)
    {
        auto file = PresetManager::getPresetsFolder().getChildFile (name + ".zgpreset");
        if (file.existsAsFile())
            return; // não sobrescreve se o utilizador já tem um preset com este nome

        juce::XmlElement xml (state.state.getType());
        for (auto& v : values)
            xml.setAttribute (v.first, (double) v.second);
        xml.writeTo (file);
    }
}

PresetManager::PresetManager (juce::AudioProcessorValueTreeState& stateToUse) : state (stateToUse)
{
    ensureFactoryPresets();
}

void PresetManager::ensureFactoryPresets()
{
    auto marker = getPresetsFolder().getChildFile (".factory_seeded");
    if (marker.existsAsFile())
        return; // já foram criados uma vez — não repor se o utilizador apagar algum

    writeFactoryPreset (state, "Piano Clean", {
        { "samplerAttack", 5 }, { "samplerDecay", 300 }, { "samplerSustain", 80 }, { "samplerRelease", 400 },
        { "grainSize", 100 }, { "grainDensity", 20 }, { "grainPan", 30 }, { "granularMix", 0 },
        { "filterCutoff", 20000 }, { "filterResonance", 0 },
        { "satDrive", 0 }, { "satMix", 0 },
        { "delayMix", 0 },
        { "reverbSize", 30 }, { "reverbDamping", 60 }, { "reverbMix", 15 }
    });

    writeFactoryPreset (state, "Bass Deep", {
        { "samplerAttack", 2 }, { "samplerDecay", 150 }, { "samplerSustain", 90 }, { "samplerRelease", 100 },
        { "samplerPitch", -12 }, { "granularMix", 0 },
        { "filterCutoff", 800 }, { "filterResonance", 20 },
        { "satMode", 1 }, { "satDrive", 30 }, { "satMix", 40 },
        { "delayMix", 0 }, { "reverbMix", 0 }
    });

    writeFactoryPreset (state, "808 Punch", {
        { "samplerAttack", 1 }, { "samplerDecay", 250 }, { "samplerSustain", 60 }, { "samplerRelease", 200 },
        { "samplerPitch", -5 }, { "granularMix", 0 },
        { "filterCutoff", 4000 }, { "filterResonance", 10 },
        { "satMode", 2 }, { "satDrive", 40 }, { "satMix", 50 },
        { "delayMix", 0 }, { "reverbMix", 0 }
    });

    writeFactoryPreset (state, "Keys Warm", {
        { "samplerAttack", 8 }, { "samplerDecay", 400 }, { "samplerSustain", 70 }, { "samplerRelease", 600 },
        { "grainSize", 150 }, { "grainDensity", 15 }, { "grainPositionRandom", 10 }, { "grainPitchRandom", 5 },
        { "grainPan", 40 }, { "granularMix", 15 },
        { "filterCutoff", 12000 }, { "filterResonance", 5 },
        { "satMode", 1 }, { "satDrive", 15 }, { "satMix", 20 },
        { "delayTime", 400 }, { "delayFeedback", 15 }, { "delayMix", 10 },
        { "reverbSize", 45 }, { "reverbDamping", 50 }, { "reverbMix", 25 }
    });

    writeFactoryPreset (state, "Pad Atmospheric", {
        { "samplerAttack", 800 }, { "samplerDecay", 1000 }, { "samplerSustain", 100 }, { "samplerRelease", 2000 },
        { "grainSize", 250 }, { "grainDensity", 40 }, { "grainPosition", 20 }, { "grainPositionRandom", 60 },
        { "grainPitchRandom", 20 }, { "grainPan", 80 }, { "granularMix", 80 },
        { "filterCutoff", 8000 }, { "filterResonance", 10 },
        { "satMode", 1 }, { "satDrive", 10 }, { "satMix", 15 },
        { "delayTime", 600 }, { "delayFeedback", 45 }, { "delayMix", 35 },
        { "reverbSize", 85 }, { "reverbDamping", 30 }, { "reverbMix", 60 }
    });

    writeFactoryPreset (state, "Lo-Fi Dust", {
        { "samplerAttack", 20 }, { "samplerDecay", 300 }, { "samplerSustain", 70 }, { "samplerRelease", 500 },
        { "samplerFineTune", -8 },
        { "grainSize", 60 }, { "grainDensity", 25 }, { "grainPosition", 10 }, { "grainPositionRandom", 30 },
        { "grainPitchRandom", 15 }, { "grainPan", 50 }, { "granularMix", 35 },
        { "filterCutoff", 3500 }, { "filterResonance", 15 },
        { "satMode", 0 }, { "satDrive", 55 }, { "satMix", 60 },
        { "delayTime", 250 }, { "delayFeedback", 25 }, { "delayMix", 20 },
        { "reverbSize", 40 }, { "reverbDamping", 70 }, { "reverbMix", 20 }
    });

    marker.create();
}

juce::File PresetManager::getPresetsFolder()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                 .getChildFile ("ZenithGranular")
                 .getChildFile ("Presets");

    if (! dir.exists())
        dir.createDirectory();

    return dir;
}

juce::StringArray PresetManager::getAllPresets() const
{
    juce::StringArray names;
    for (auto& f : getPresetsFolder().findChildFiles (juce::File::findFiles, false, "*.zgpreset"))
        names.add (f.getFileNameWithoutExtension());
    names.sort (true);
    return names;
}

void PresetManager::savePreset (const juce::String& name)
{
    if (name.isEmpty())
        return;

    auto file = getPresetsFolder().getChildFile (name + ".zgpreset");
    auto currentState = state.copyState();
    std::unique_ptr<juce::XmlElement> xml (currentState.createXml());
    if (xml != nullptr)
        xml->writeTo (file);
}

void PresetManager::loadPreset (const juce::String& name)
{
    auto file = getPresetsFolder().getChildFile (name + ".zgpreset");
    if (! file.existsAsFile())
        return;

    if (auto xml = juce::XmlDocument::parse (file))
        if (xml->hasTagName (state.state.getType()))
            state.replaceState (juce::ValueTree::fromXml (*xml));
}

void PresetManager::deletePreset (const juce::String& name)
{
    getPresetsFolder().getChildFile (name + ".zgpreset").deleteFile();
}
