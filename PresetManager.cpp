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

juce::File PresetManager::getPresetsFolder()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                 .getChildFile ("ZenithGranular")
                 .getChildFile ("Presets");

    if (! dir.exists())
        dir.createDirectory();

    return dir;
}

juce::File PresetManager::getFavoritesFile()
{
    return getPresetsFolder().getChildFile ("favorites.txt");
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

    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr)
        return;

    // Aplica cada parâmetro através de getParameterAsValue — o mesmo
    // mecanismo já usado (e confirmado a funcionar) pelos Macros. Evitamos
    // "replaceState" porque presets de fábrica só têm um subconjunto de
    // parâmetros, e o replaceState não estava a refletir isso nos knobs.
    for (int i = 0; i < xml->getNumAttributes(); ++i)
    {
        const auto id = xml->getAttributeName (i);
        if (id == "samplePath")
            continue; // tratado à parte, logo a seguir

        if (state.getParameter (id) != nullptr)
            state.getParameterAsValue (id).setValue (xml->getAttributeValue (i).getDoubleValue());
    }

    if (xml->hasAttribute ("samplePath"))
        state.state.setProperty ("samplePath", xml->getStringAttribute ("samplePath"), nullptr);
}

void PresetManager::deletePreset (const juce::String& name)
{
    getPresetsFolder().getChildFile (name + ".zgpreset").deleteFile();
    setFavorite (name, false);
}

bool PresetManager::isFavorite (const juce::String& name) const
{
    return getFavoritePresets().contains (name);
}

void PresetManager::setFavorite (const juce::String& name, bool shouldBeFavorite)
{
    auto favorites = getFavoritePresets();
    if (shouldBeFavorite)
        favorites.addIfNotAlreadyThere (name);
    else
        favorites.removeString (name);

    getFavoritesFile().replaceWithText (favorites.joinIntoString ("\n"));
}

juce::StringArray PresetManager::getFavoritePresets() const
{
    auto file = getFavoritesFile();
    if (! file.existsAsFile())
        return {};

    juce::StringArray lines;
    file.readLines (lines);
    lines.removeEmptyStrings();
    return lines;
}

void PresetManager::ensureFactoryPresets()
{
    auto marker = getPresetsFolder().getChildFile (".factory_seeded");
    if (marker.existsAsFile())
        return; // já foram criados uma vez — não repor se o utilizador apagar algum

    // --- PIANO (3 variações) ---
    writeFactoryPreset (state, "Piano Clean", {
        { "samplerAttack", 5 }, { "samplerDecay", 300 }, { "samplerSustain", 80 }, { "samplerRelease", 400 },
        { "filterCutoff", 20000 }, { "filterResonance", 0 },
        { "reverbSize", 30 }, { "reverbDamping", 60 }, { "reverbMix", 15 }
    });
    writeFactoryPreset (state, "Piano Bright", {
        { "samplerAttack", 3 }, { "samplerDecay", 250 }, { "samplerSustain", 85 }, { "samplerRelease", 350 },
        { "filterCutoff", 20000 }, { "filterResonance", 0 },
        { "reverbSize", 40 }, { "reverbDamping", 40 }, { "reverbMix", 25 }
    });
    writeFactoryPreset (state, "Piano Soft", {
        { "samplerAttack", 40 }, { "samplerDecay", 500 }, { "samplerSustain", 90 }, { "samplerRelease", 700 },
        { "filterCutoff", 9000 }, { "filterResonance", 0 },
        { "satMode", 1 }, { "satDrive", 10 }, { "satMix", 15 },
        { "reverbSize", 50 }, { "reverbDamping", 70 }, { "reverbMix", 30 }
    });

    // --- BASS (3 variações) ---
    writeFactoryPreset (state, "Bass Deep", {
        { "samplerAttack", 2 }, { "samplerDecay", 150 }, { "samplerSustain", 90 }, { "samplerRelease", 100 },
        { "samplerPitch", -12 },
        { "filterCutoff", 800 }, { "filterResonance", 20 },
        { "satMode", 1 }, { "satDrive", 30 }, { "satMix", 40 }
    });
    writeFactoryPreset (state, "Bass Bright", {
        { "samplerAttack", 2 }, { "samplerDecay", 180 }, { "samplerSustain", 85 }, { "samplerRelease", 120 },
        { "samplerPitch", -7 },
        { "filterCutoff", 2500 }, { "filterResonance", 15 },
        { "satMode", 2 }, { "satDrive", 45 }, { "satMix", 55 }
    });
    writeFactoryPreset (state, "Bass Sub", {
        { "samplerAttack", 3 }, { "samplerDecay", 200 }, { "samplerSustain", 95 }, { "samplerRelease", 150 },
        { "samplerPitch", -19 },
        { "filterCutoff", 400 }, { "filterResonance", 10 },
        { "satMode", 1 }, { "satDrive", 15 }, { "satMix", 20 }
    });

    // --- 808 (3 variações) ---
    writeFactoryPreset (state, "808 Punch", {
        { "samplerAttack", 1 }, { "samplerDecay", 250 }, { "samplerSustain", 60 }, { "samplerRelease", 200 },
        { "samplerPitch", -5 },
        { "filterCutoff", 4000 }, { "filterResonance", 10 },
        { "satMode", 2 }, { "satDrive", 40 }, { "satMix", 50 }
    });
    writeFactoryPreset (state, "808 Sub", {
        { "samplerAttack", 1 }, { "samplerDecay", 400 }, { "samplerSustain", 70 }, { "samplerRelease", 500 },
        { "samplerPitch", -12 },
        { "filterCutoff", 1200 }, { "filterResonance", 5 },
        { "satMode", 1 }, { "satDrive", 15 }, { "satMix", 20 }
    });
    writeFactoryPreset (state, "808 Distorted", {
        { "samplerAttack", 1 }, { "samplerDecay", 150 }, { "samplerSustain", 50 }, { "samplerRelease", 100 },
        { "samplerPitch", -3 },
        { "filterCutoff", 6000 }, { "filterResonance", 20 },
        { "satMode", 2 }, { "satDrive", 75 }, { "satMix", 80 }
    });

    // --- KEYS (3 variações) ---
    writeFactoryPreset (state, "Keys Warm", {
        { "samplerAttack", 8 }, { "samplerDecay", 400 }, { "samplerSustain", 70 }, { "samplerRelease", 600 },
        { "grainSize", 150 }, { "grainDensity", 15 }, { "grainPositionRandom", 10 }, { "grainPitchRandom", 5 },
        { "grainPan", 40 }, { "granularMix", 15 },
        { "filterCutoff", 12000 }, { "filterResonance", 5 },
        { "satMode", 1 }, { "satDrive", 15 }, { "satMix", 20 },
        { "delayTime", 400 }, { "delayFeedback", 15 }, { "delayMix", 10 },
        { "reverbSize", 45 }, { "reverbDamping", 50 }, { "reverbMix", 25 }
    });
    writeFactoryPreset (state, "Keys Bright", {
        { "samplerAttack", 5 }, { "samplerDecay", 350 }, { "samplerSustain", 75 }, { "samplerRelease", 500 },
        { "grainSize", 100 }, { "grainDensity", 25 }, { "grainPositionRandom", 15 }, { "grainPitchRandom", 8 },
        { "grainPan", 50 }, { "granularMix", 25 },
        { "filterCutoff", 16000 }, { "filterResonance", 5 },
        { "satMode", 0 }, { "satDrive", 10 }, { "satMix", 10 },
        { "delayTime", 350 }, { "delayFeedback", 20 }, { "delayMix", 15 },
        { "reverbSize", 35 }, { "reverbDamping", 45 }, { "reverbMix", 20 }
    });
    writeFactoryPreset (state, "Keys Dreamy", {
        { "samplerAttack", 100 }, { "samplerDecay", 600 }, { "samplerSustain", 90 }, { "samplerRelease", 1200 },
        { "grainSize", 200 }, { "grainDensity", 30 }, { "grainPosition", 10 }, { "grainPositionRandom", 40 },
        { "grainPitchRandom", 10 }, { "grainPan", 70 }, { "granularMix", 55 },
        { "filterCutoff", 10000 }, { "filterResonance", 5 },
        { "delayTime", 500 }, { "delayFeedback", 35 }, { "delayMix", 30 },
        { "reverbSize", 70 }, { "reverbDamping", 40 }, { "reverbMix", 45 }
    });

    // --- PAD (2 variações) ---
    writeFactoryPreset (state, "Pad Atmospheric", {
        { "samplerAttack", 800 }, { "samplerDecay", 1000 }, { "samplerSustain", 100 }, { "samplerRelease", 2000 },
        { "grainSize", 250 }, { "grainDensity", 40 }, { "grainPosition", 20 }, { "grainPositionRandom", 60 },
        { "grainPitchRandom", 20 }, { "grainPan", 80 }, { "granularMix", 80 },
        { "filterCutoff", 8000 }, { "filterResonance", 10 },
        { "satMode", 1 }, { "satDrive", 10 }, { "satMix", 15 },
        { "delayTime", 600 }, { "delayFeedback", 45 }, { "delayMix", 35 },
        { "reverbSize", 85 }, { "reverbDamping", 30 }, { "reverbMix", 60 }
    });
    writeFactoryPreset (state, "Pad Dark", {
        { "samplerAttack", 1000 }, { "samplerDecay", 1200 }, { "samplerSustain", 100 }, { "samplerRelease", 2500 },
        { "grainSize", 300 }, { "grainDensity", 35 }, { "grainPosition", 30 }, { "grainPositionRandom", 50 },
        { "grainPitchRandom", 15 }, { "grainPan", 90 }, { "granularMix", 85 },
        { "filterCutoff", 4000 }, { "filterResonance", 15 },
        { "satMode", 1 }, { "satDrive", 20 }, { "satMix", 25 },
        { "delayTime", 700 }, { "delayFeedback", 50 }, { "delayMix", 40 },
        { "reverbSize", 90 }, { "reverbDamping", 20 }, { "reverbMix", 70 }
    });

    // --- LO-FI (2 variações) ---
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
    writeFactoryPreset (state, "Lo-Fi Wobble", {
        { "samplerAttack", 15 }, { "samplerDecay", 350 }, { "samplerSustain", 65 }, { "samplerRelease", 450 },
        { "samplerFineTune", -20 },
        { "grainSize", 45 }, { "grainDensity", 30 }, { "grainPositionRandom", 40 },
        { "grainPitchRandom", 25 }, { "grainPan", 40 }, { "granularMix", 45 },
        { "filterCutoff", 2800 }, { "filterResonance", 20 },
        { "satMode", 0 }, { "satDrive", 65 }, { "satMix", 70 },
        { "delayTime", 200 }, { "delayFeedback", 30 }, { "delayMix", 25 },
        { "reverbSize", 35 }, { "reverbDamping", 75 }, { "reverbMix", 15 }
    });

    marker.create();
}
