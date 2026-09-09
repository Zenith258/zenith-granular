#include "PresetManager.h"
#include <vector>
#include <utility>

namespace
{
    void writeFactoryPreset (juce::AudioProcessorValueTreeState& state, const juce::String& name,
                              const std::vector<std::pair<juce::String, float>>& values)
    {
        auto file = PresetManager::getPresetsFolder().getChildFile (name + ".zgpreset");
        if (file.existsAsFile())
            return;

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

juce::File PresetManager::getRatingsFile() { return getPresetsFolder().getChildFile ("ratings.txt"); }
juce::File PresetManager::getFactoryManifestFile() { return getPresetsFolder().getChildFile (".factory_presets.txt"); }

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

    for (int i = 0; i < xml->getNumAttributes(); ++i)
    {
        const auto id = xml->getAttributeName (i);
        if (id == "samplePath")
            continue;

        if (state.getParameter (id) != nullptr)
            state.getParameterAsValue (id).setValue (xml->getAttributeValue (i).getDoubleValue());
    }

    if (xml->hasAttribute ("samplePath"))
        state.state.setProperty ("samplePath", xml->getStringAttribute ("samplePath"), nullptr);
}

bool PresetManager::isFactoryPreset (const juce::String& name) const
{
    auto file = getFactoryManifestFile();
    if (! file.existsAsFile())
        return false;
    juce::StringArray lines;
    file.readLines (lines);
    return lines.contains (name);
}

void PresetManager::deletePreset (const juce::String& name)
{
    if (isFactoryPreset (name))
        return; // presets nativos não podem ser apagados

    getPresetsFolder().getChildFile (name + ".zgpreset").deleteFile();
    setRating (name, 0);
}

int PresetManager::getRating (const juce::String& name) const
{
    auto file = getRatingsFile();
    if (! file.existsAsFile())
        return 0;

    juce::StringArray lines;
    file.readLines (lines);
    for (auto& line : lines)
    {
        const auto n = line.upToFirstOccurrenceOf ("|", false, false);
        if (n == name)
            return line.fromFirstOccurrenceOf ("|", false, false).getIntValue();
    }
    return 0;
}

void PresetManager::setRating (const juce::String& name, int stars)
{
    stars = juce::jlimit (0, 3, stars);

    auto file = getRatingsFile();
    juce::StringArray lines;
    if (file.existsAsFile())
        file.readLines (lines);

    bool found = false;
    for (auto& line : lines)
    {
        const auto n = line.upToFirstOccurrenceOf ("|", false, false);
        if (n == name)
        {
            line = name + "|" + juce::String (stars);
            found = true;
            break;
        }
    }
    if (! found)
        lines.add (name + "|" + juce::String (stars));

    file.replaceWithText (lines.joinIntoString ("\n"));
}

juce::StringArray PresetManager::getFavoritePresets() const
{
    juce::StringArray result;
    auto file = getRatingsFile();
    if (! file.existsAsFile())
        return result;

    juce::StringArray lines;
    file.readLines (lines);
    for (auto& line : lines)
    {
        const auto n = line.upToFirstOccurrenceOf ("|", false, false);
        const auto r = line.fromFirstOccurrenceOf ("|", false, false).getIntValue();
        if (r > 0 && n.isNotEmpty())
            result.add (n);
    }
    result.sort (true);
    return result;
}

void PresetManager::ensureFactoryPresets()
{
    static const std::vector<juce::String> factoryNames = {
        "Piano Clean", "Piano Bright", "Piano Soft", "Piano Dark", "Piano Granular",
        "Bass Deep", "Bass Bright", "Bass Sub", "Bass Distorted", "Bass Granular",
        "808 Punch", "808 Sub", "808 Distorted", "808 Clean", "808 Wide",
        "Keys Warm", "Keys Bright", "Keys Dreamy", "Keys Granular", "Keys Vintage",
        "Pad Atmospheric", "Pad Dark", "Pad Bright", "Pad Drone",
        "Lo-Fi Dust", "Lo-Fi Wobble", "Lo-Fi Crushed", "Lo-Fi Ambient"
    };

    // Garante o manifesto mesmo que os ficheiros já existam de uma versão
    // anterior (para proteger contra apagar quem já tinha instalado antes).
    auto manifestFile = getFactoryManifestFile();
    if (! manifestFile.existsAsFile())
    {
        juce::StringArray arr;
        for (auto& n : factoryNames)
            arr.add (n);
        manifestFile.replaceWithText (arr.joinIntoString ("\n"));
    }

    auto marker = getPresetsFolder().getChildFile (".factory_seeded");
    if (marker.existsAsFile())
        return;

    // --- PIANO ---
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
    writeFactoryPreset (state, "Piano Dark", {
        { "samplerAttack", 15 }, { "samplerDecay", 400 }, { "samplerSustain", 75 }, { "samplerRelease", 500 },
        { "filterCutoff", 6000 }, { "filterResonance", 5 },
        { "satMode", 1 }, { "satDrive", 10 }, { "satMix", 15 },
        { "reverbSize", 55 }, { "reverbDamping", 60 }, { "reverbMix", 30 }
    });
    writeFactoryPreset (state, "Piano Granular", {
        { "samplerAttack", 20 }, { "samplerDecay", 350 }, { "samplerSustain", 80 }, { "samplerRelease", 600 },
        { "grainSize", 120 }, { "grainDensity", 18 }, { "grainPositionRandom", 8 }, { "grainPitchRandom", 4 },
        { "grainPan", 35 }, { "granularMix", 20 },
        { "filterCutoff", 15000 },
        { "reverbSize", 35 }, { "reverbDamping", 55 }, { "reverbMix", 20 }
    });

    // --- BASS ---
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
    writeFactoryPreset (state, "Bass Distorted", {
        { "samplerAttack", 1 }, { "samplerDecay", 130 }, { "samplerSustain", 80 }, { "samplerRelease", 90 },
        { "samplerPitch", -10 },
        { "filterCutoff", 1800 }, { "filterResonance", 25 },
        { "satMode", 2 }, { "satDrive", 70 }, { "satMix", 75 }
    });
    writeFactoryPreset (state, "Bass Granular", {
        { "samplerAttack", 5 }, { "samplerDecay", 200 }, { "samplerSustain", 85 }, { "samplerRelease", 150 },
        { "samplerPitch", -10 },
        { "grainSize", 80 }, { "grainDensity", 22 }, { "granularMix", 25 },
        { "filterCutoff", 1500 }, { "filterResonance", 15 },
        { "satMode", 1 }, { "satDrive", 25 }, { "satMix", 30 }
    });

    // --- 808 ---
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
    writeFactoryPreset (state, "808 Clean", {
        { "samplerAttack", 1 }, { "samplerDecay", 300 }, { "samplerSustain", 65 }, { "samplerRelease", 250 },
        { "samplerPitch", -7 },
        { "filterCutoff", 5000 }, { "filterResonance", 5 },
        { "satMode", 0 }, { "satDrive", 5 }, { "satMix", 10 }
    });
    writeFactoryPreset (state, "808 Wide", {
        { "samplerAttack", 1 }, { "samplerDecay", 280 }, { "samplerSustain", 60 }, { "samplerRelease", 300 },
        { "samplerPitch", -6 },
        { "filterCutoff", 4500 }, { "filterResonance", 12 },
        { "satMode", 1 }, { "satDrive", 20 }, { "satMix", 25 },
        { "delayTime", 280 }, { "delayFeedback", 20 }, { "delayMix", 18 },
        { "reverbSize", 50 }, { "reverbDamping", 50 }, { "reverbMix", 25 }
    });

    // --- KEYS ---
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
    writeFactoryPreset (state, "Keys Granular", {
        { "samplerAttack", 10 }, { "samplerDecay", 400 }, { "samplerSustain", 75 }, { "samplerRelease", 700 },
        { "grainSize", 180 }, { "grainDensity", 28 }, { "grainPosition", 5 }, { "grainPositionRandom", 25 },
        { "grainPitchRandom", 12 }, { "grainPan", 60 }, { "granularMix", 40 },
        { "filterCutoff", 11000 },
        { "delayTime", 420 }, { "delayFeedback", 25 }, { "delayMix", 18 },
        { "reverbSize", 55 }, { "reverbDamping", 45 }, { "reverbMix", 30 }
    });
    writeFactoryPreset (state, "Keys Vintage", {
        { "samplerAttack", 12 }, { "samplerDecay", 450 }, { "samplerSustain", 72 }, { "samplerRelease", 550 },
        { "filterCutoff", 8000 }, { "filterResonance", 8 },
        { "satMode", 1 }, { "satDrive", 20 }, { "satMix", 25 },
        { "reverbSize", 50 }, { "reverbDamping", 65 }, { "reverbMix", 28 }
    });

    // --- PAD ---
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
    writeFactoryPreset (state, "Pad Bright", {
        { "samplerAttack", 600 }, { "samplerDecay", 900 }, { "samplerSustain", 100 }, { "samplerRelease", 1800 },
        { "grainSize", 220 }, { "grainDensity", 38 }, { "grainPosition", 15 }, { "grainPositionRandom", 45 },
        { "grainPitchRandom", 12 }, { "grainPan", 75 }, { "granularMix", 70 },
        { "filterCutoff", 14000 }, { "filterResonance", 8 },
        { "delayTime", 550 }, { "delayFeedback", 40 }, { "delayMix", 30 },
        { "reverbSize", 75 }, { "reverbDamping", 25 }, { "reverbMix", 55 }
    });
    writeFactoryPreset (state, "Pad Drone", {
        { "samplerAttack", 2000 }, { "samplerDecay", 1500 }, { "samplerSustain", 100 }, { "samplerRelease", 3000 },
        { "grainSize", 350 }, { "grainDensity", 45 }, { "grainPosition", 40 }, { "grainPositionRandom", 70 },
        { "grainPitchRandom", 25 }, { "grainPan", 95 }, { "granularMix", 90 },
        { "filterCutoff", 3000 }, { "filterResonance", 20 },
        { "satMode", 1 }, { "satDrive", 25 }, { "satMix", 30 },
        { "delayTime", 800 }, { "delayFeedback", 55 }, { "delayMix", 45 },
        { "reverbSize", 95 }, { "reverbDamping", 15 }, { "reverbMix", 75 }
    });

    // --- LO-FI ---
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
    writeFactoryPreset (state, "Lo-Fi Crushed", {
        { "samplerAttack", 10 }, { "samplerDecay", 250 }, { "samplerSustain", 60 }, { "samplerRelease", 400 },
        { "samplerFineTune", -12 },
        { "grainSize", 40 }, { "grainDensity", 28 }, { "grainPositionRandom", 35 },
        { "grainPitchRandom", 20 }, { "grainPan", 45 }, { "granularMix", 40 },
        { "filterCutoff", 2200 }, { "filterResonance", 25 },
        { "satMode", 0 }, { "satDrive", 80 }, { "satMix", 85 },
        { "delayTime", 220 }, { "delayFeedback", 28 }, { "delayMix", 22 },
        { "reverbSize", 30 }, { "reverbDamping", 80 }, { "reverbMix", 12 }
    });
    writeFactoryPreset (state, "Lo-Fi Ambient", {
        { "samplerAttack", 30 }, { "samplerDecay", 400 }, { "samplerSustain", 75 }, { "samplerRelease", 700 },
        { "samplerFineTune", -6 },
        { "grainSize", 70 }, { "grainDensity", 22 }, { "grainPosition", 15 }, { "grainPositionRandom", 25 },
        { "grainPitchRandom", 10 }, { "grainPan", 55 }, { "granularMix", 40 },
        { "filterCutoff", 4200 }, { "filterResonance", 12 },
        { "satMode", 1 }, { "satDrive", 35 }, { "satMix", 40 },
        { "delayTime", 320 }, { "delayFeedback", 35 }, { "delayMix", 30 },
        { "reverbSize", 60 }, { "reverbDamping", 55 }, { "reverbMix", 40 }
    });

    marker.create();
}
