#include "SamplerEngine.h"

SamplerEngine::SamplerEngine()
{
    formatManager.registerBasicFormats(); // WAV, AIFF
    formatManager.registerFormat (new juce::FlacAudioFormat(), false);
    formatManager.registerFormat (new juce::OggVorbisAudioFormat(), false);

   #if JUCE_WINDOWS
    // No Windows, isto usa os codecs do próprio sistema (Media Foundation)
    // para decodificar MP3, M4A/AAC e WMA — não precisa de bibliotecas extra.
    formatManager.registerFormat (new juce::WindowsMediaAudioFormat(), false);
   #endif

    for (int i = 0; i < numVoices; ++i)
        synth.addVoice (new ZenithSamplerVoice (voiceParams));
}

void SamplerEngine::prepare (double sampleRate, int /*samplesPerBlock*/)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
}

void SamplerEngine::loadSample (const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    if (reader == nullptr)
        return; // ficheiro inválido ou formato não suportado — ignora sem crashar

    juce::AudioBuffer<float> newBuffer ((int) reader->numChannels, (int) reader->lengthInSamples);
    reader->read (&newBuffer, 0, (int) reader->lengthInSamples, 0, true, true);

    // Nota raiz fixa em C4 (60) por agora — mapeamento de várias zonas fica
    // para uma fase posterior. Um único som cobre o teclado inteiro.
    synth.clearSounds();
    synth.addSound (new ZenithSamplerSound (std::move (newBuffer), reader->sampleRate, 60));
    sampleLoaded = true;
}

void SamplerEngine::renderNextBlock (juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi,
                                      int startSample, int numSamples)
{
    synth.renderNextBlock (buffer, midi, startSample, numSamples);
}
