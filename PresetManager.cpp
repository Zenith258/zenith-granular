#include "PresetManager.h"

PresetManager::PresetManager (juce::AudioProcessorValueTreeState& stateToUse) : state (stateToUse)
{
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
