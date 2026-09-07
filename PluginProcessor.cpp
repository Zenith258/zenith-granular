#include "PluginProcessor.h"
#include "PluginEditor.h"

ZenithGranularAudioProcessor::ZenithGranularAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

ZenithGranularAudioProcessor::~ZenithGranularAudioProcessor() = default;

void ZenithGranularAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    // Chamado uma vez antes do áudio começar a correr — é aqui, e não no
    // processBlock, que qualquer buffer deve ser pré-alocado (regra de CPU
    // da arquitetura: nada de alocações dinâmicas na audio thread).
    currentSampleRate = sampleRate;
}

void ZenithGranularAudioProcessor::releaseResources()
{
}

bool ZenithGranularAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Só aceitamos saída estéreo por agora.
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void ZenithGranularAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                  juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Limpa qualquer canal extra que o host possa ter alocado.
    for (auto ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    // FASE 1: ainda não geramos som. Só garantimos que o buffer sai limpo
    // (silêncio) e que as mensagens MIDI são recebidas sem crashar —
    // isto é o que prova que o "instrumento" está corretamente registado.
    buffer.clear();

    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();
        juce::ignoreUnused (message);
        // Na FASE 2: noteOn/noteOff vão disparar o SamplerEngine aqui.
    }
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

void ZenithGranularAudioProcessor::getStateInformation (juce::MemoryBlock& /*destData*/)
{
    // Na FASE 7 (Presets), isto vai serializar o AudioProcessorValueTreeState.
}

void ZenithGranularAudioProcessor::setStateInformation (const void* /*data*/, int /*sizeInBytes*/)
{
}

// Ponto de entrada que o JUCE usa para criar o processor.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZenithGranularAudioProcessor();
}
