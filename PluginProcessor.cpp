#include "PluginProcessor.h"
#include "PluginEditor.h"

ZenithGranularAudioProcessor::ZenithGranularAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    // Liga a engine aos valores ao vivo da APVTS — cada voz vai ler estes
    // ponteiros diretamente, sem cópias, sem alocações na audio thread.
    samplerEngine.connectParameters ({
        apvts.getRawParameterValue ("samplerAttack"),
        apvts.getRawParameterValue ("samplerDecay"),
        apvts.getRawParameterValue ("samplerSustain"),
        apvts.getRawParameterValue ("samplerRelease"),
        apvts.getRawParameterValue ("samplerPitch"),
        apvts.getRawParameterValue ("samplerFineTune"),
        apvts.getRawParameterValue ("grainSize"),
        apvts.getRawParameterValue ("grainDensity"),
        apvts.getRawParameterValue ("grainPosition"),
        apvts.getRawParameterValue ("grainPositionRandom"),
        apvts.getRawParameterValue ("grainPitch"),
        apvts.getRawParameterValue ("grainPitchRandom"),
        apvts.getRawParameterValue ("grainPan"),
        apvts.getRawParameterValue ("granularMix")
    });

    filterEngine.connectParameters ({
        apvts.getRawParameterValue ("filterType"),
        apvts.getRawParameterValue ("filterCutoff"),
        apvts.getRawParameterValue ("filterResonance")
    });

    saturationEngine.connectParameters ({
        apvts.getRawParameterValue ("satMode"),
        apvts.getRawParameterValue ("satDrive"),
        apvts.getRawParameterValue ("satMix")
    });

    delayEngine.connectParameters ({
        apvts.getRawParameterValue ("delayTime"),
        apvts.getRawParameterValue ("delayFeedback"),
        apvts.getRawParameterValue ("delayMix")
    });

    reverbEngine.connectParameters ({
        apvts.getRawParameterValue ("reverbSize"),
        apvts.getRawParameterValue ("reverbDamping"),
        apvts.getRawParameterValue ("reverbMix")
    });
}

ZenithGranularAudioProcessor::~ZenithGranularAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout ZenithGranularAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "samplerAttack", 1 }, "Attack",
        juce::NormalisableRange<float> (0.0f, 5000.0f, 1.0f), 5.0f, "ms"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "samplerDecay", 1 }, "Decay",
        juce::NormalisableRange<float> (0.0f, 5000.0f, 1.0f), 200.0f, "ms"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "samplerSustain", 1 }, "Sustain",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f, "%"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "samplerRelease", 1 }, "Release",
        juce::NormalisableRange<float> (0.0f, 5000.0f, 1.0f), 300.0f, "ms"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "samplerPitch", 1 }, "Pitch",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.01f), 0.0f, "st"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "samplerFineTune", 1 }, "Fine Tune",
        juce::NormalisableRange<float> (-100.0f, 100.0f, 0.1f), 0.0f, "cents"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "grainSize", 1 }, "Grain Size",
        juce::NormalisableRange<float> (10.0f, 500.0f, 1.0f), 80.0f, "ms"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "grainDensity", 1 }, "Grain Density",
        juce::NormalisableRange<float> (1.0f, 100.0f, 0.1f), 20.0f, "/s"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "grainPosition", 1 }, "Grain Position",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, "%"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "grainPositionRandom", 1 }, "Position Random",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, "%"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "grainPitch", 1 }, "Grain Pitch",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.01f), 0.0f, "st"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "grainPitchRandom", 1 }, "Pitch Random",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, "%"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "grainPan", 1 }, "Grain Pan Spread",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f, "%"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "granularMix", 1 }, "Granular Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, "%"));

    // --- Filtro ---
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "filterType", 1 }, "Filter Type",
        juce::StringArray { "Low-Pass", "High-Pass", "Band-Pass" }, 0));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "filterCutoff", 1 }, "Filter Cutoff",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.3f), 20000.0f, "Hz"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "filterResonance", 1 }, "Filter Resonance",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, "%"));

    // --- Saturação ---
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "satMode", 1 }, "Saturation Mode",
        juce::StringArray { "Soft Clip", "Tape", "Tube" }, 0));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "satDrive", 1 }, "Saturation Drive",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, "%"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "satMix", 1 }, "Saturation Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, "%"));

    // --- Delay ---
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "delayTime", 1 }, "Delay Time",
        juce::NormalisableRange<float> (1.0f, 2000.0f, 1.0f), 300.0f, "ms"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "delayFeedback", 1 }, "Delay Feedback",
        juce::NormalisableRange<float> (0.0f, 95.0f, 0.1f), 30.0f, "%"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "delayMix", 1 }, "Delay Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, "%"));

    // --- Reverb ---
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "reverbSize", 1 }, "Reverb Size",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f, "%"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "reverbDamping", 1 }, "Reverb Damping",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f, "%"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "reverbMix", 1 }, "Reverb Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, "%"));

    return { params.begin(), params.end() };
}

void ZenithGranularAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    samplerEngine.prepare (sampleRate, samplesPerBlock);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) getTotalNumOutputChannels();

    filterEngine.prepare (spec);
    delayEngine.prepare (spec);
    reverbEngine.prepare (spec);
}

void ZenithGranularAudioProcessor::releaseResources()
{
}

bool ZenithGranularAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void ZenithGranularAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                  juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    for (auto ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    buffer.clear();
    samplerEngine.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    filterEngine.process (buffer);
    saturationEngine.process (buffer);
    delayEngine.process (buffer);
    reverbEngine.process (buffer);
}

juce::AudioProcessorEditor* ZenithGranularAudioProcessor::createEditor()
{
    return new ZenithGranularAudioProcessorEditor (*this);
}

bool ZenithGranularAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String ZenithGranularAudioProcessor::getName() const
{
    return "Zenith Granular";
}

bool ZenithGranularAudioProcessor::acceptsMidi() const { return true; }
bool ZenithGranularAudioProcessor::producesMidi() const { return false; }
bool ZenithGranularAudioProcessor::isMidiEffect() const { return false; }
double ZenithGranularAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int ZenithGranularAudioProcessor::getNumPrograms() { return 1; }
int ZenithGranularAudioProcessor::getCurrentProgram() { return 0; }
void ZenithGranularAudioProcessor::setCurrentProgram (int) {}
const juce::String ZenithGranularAudioProcessor::getProgramName (int) { return {}; }
void ZenithGranularAudioProcessor::changeProgramName (int, const juce::String&) {}

void ZenithGranularAudioProcessor::loadSample (const juce::File& file)
{
    samplerEngine.loadSample (file);
    // Guarda o caminho no estado do plugin — assim, quando o FL Studio
    // guardar o projeto (.flp) e reabrir depois, sabemos qual sample recarregar.
    apvts.state.setProperty ("samplePath", file.getFullPathName(), nullptr);
}

void ZenithGranularAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    if (xml != nullptr)
        copyXmlToBinary (*xml, destData);
}

void ZenithGranularAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
    {
        apvts.replaceState (juce::ValueTree::fromXml (*xml));

        // Se o projeto já tinha um sample carregado, recarrega-o agora
        // (ex.: ao reabrir um .flp que já usava este plugin).
        if (apvts.state.hasProperty ("samplePath"))
        {
            juce::File file (apvts.state.getProperty ("samplePath").toString());
            if (file.existsAsFile())
                samplerEngine.loadSample (file);
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZenithGranularAudioProcessor();
}
