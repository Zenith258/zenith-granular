#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <atomic>

/**
    Ponteiros para os valores ao vivo da APVTS (Attack/Decay/Sustain/Release,
    Pitch, Fine Tune). Cada voz consulta isto a cada nota tocada, para reagir
    em tempo real aos knobs da UI sem precisar de recriar nada.
*/
struct SamplerVoiceParams
{
    std::atomic<float>* attack   = nullptr; // ms
    std::atomic<float>* decay    = nullptr; // ms
    std::atomic<float>* sustain  = nullptr; // %
    std::atomic<float>* release  = nullptr; // ms
    std::atomic<float>* pitch    = nullptr; // semitons
    std::atomic<float>* fineTune = nullptr; // cents
};

/** O "som" carregado — o buffer de áudio do sample e a nota raiz (C4/60). */
class ZenithSamplerSound : public juce::SynthesiserSound
{
public:
    ZenithSamplerSound (juce::AudioBuffer<float> bufferToUse, double sourceSampleRate, int midiRootNote)
        : data (std::move (bufferToUse)), sampleRate (sourceSampleRate), rootNote (midiRootNote)
    {
    }

    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }

    const juce::AudioBuffer<float>& getData() const { return data; }
    double getSourceSampleRate() const { return sampleRate; }
    int getRootNote() const { return rootNote; }

private:
    juce::AudioBuffer<float> data;
    double sampleRate;
    int rootNote;
};

/**
    Uma voz do sampler: reproduz o buffer do ZenithSamplerSound com
    interpolação linear (para pitch fracionário), aplicando ADSR e pitch/fine
    tune lidos ao vivo de SamplerVoiceParams.
*/
class ZenithSamplerVoice : public juce::SynthesiserVoice
{
public:
    explicit ZenithSamplerVoice (const SamplerVoiceParams& engineParams) : params (engineParams) {}

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<ZenithSamplerSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* s, int) override
    {
        currentSound = dynamic_cast<ZenithSamplerSound*> (s);
        if (currentSound == nullptr)
            return;

        currentVelocity = velocity;
        sourcePosition = 0.0;

        const auto semitoneOffset = (double) (midiNoteNumber - currentSound->getRootNote());
        const auto extraPitch     = params.pitch    != nullptr ? (double) params.pitch->load()    : 0.0;
        const auto fineTuneCents  = params.fineTune != nullptr ? (double) params.fineTune->load()  : 0.0;

        pitchRatio = std::pow (2.0, (semitoneOffset + extraPitch + fineTuneCents / 100.0) / 12.0)
                     * (currentSound->getSourceSampleRate() / getSampleRate());

        juce::ADSR::Parameters adsrParams;
        adsrParams.attack  = params.attack  != nullptr ? juce::jmax (0.001f, params.attack->load()  / 1000.0f) : 0.01f;
        adsrParams.decay   = params.decay   != nullptr ? juce::jmax (0.001f, params.decay->load()   / 1000.0f) : 0.1f;
        adsrParams.sustain = params.sustain != nullptr ? params.sustain->load() / 100.0f : 1.0f;
        adsrParams.release = params.release != nullptr ? juce::jmax (0.001f, params.release->load() / 1000.0f) : 0.2f;
        adsr.setSampleRate (getSampleRate());
        adsr.setParameters (adsrParams);
        adsr.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            adsr.noteOff();
        }
        else
        {
            adsr.reset();
            clearCurrentNote();
            currentSound = nullptr;
        }
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (currentSound == nullptr)
            return;

        const auto& data = currentSound->getData();
        const auto numSourceChannels = data.getNumChannels();
        const auto numSourceSamples  = data.getNumSamples();

        for (int i = 0; i < numSamples; ++i)
        {
            if (sourcePosition >= (double) (numSourceSamples - 1) || ! adsr.isActive())
            {
                clearCurrentNote();
                currentSound = nullptr;
                break;
            }

            const auto env  = adsr.getNextSample();
            const auto pos  = (int) sourcePosition;
            const auto frac = (float) (sourcePosition - (double) pos);

            for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
            {
                const auto srcChannel = juce::jmin (ch, numSourceChannels - 1);
                const auto s0 = data.getSample (srcChannel, pos);
                const auto s1 = data.getSample (srcChannel, juce::jmin (pos + 1, numSourceSamples - 1));
                const auto interpolated = s0 + frac * (s1 - s0);
                outputBuffer.addSample (ch, startSample + i, interpolated * env * currentVelocity);
            }

            sourcePosition += pitchRatio;
        }
    }

private:
    const SamplerVoiceParams& params;
    ZenithSamplerSound* currentSound = nullptr;
    double sourcePosition = 0.0;
    double pitchRatio = 1.0;
    float currentVelocity = 1.0f;
    juce::ADSR adsr;
};

/** Wrapper de alto nível: carrega um .wav e expõe render/prepare ao PluginProcessor. */
class SamplerEngine
{
public:
    SamplerEngine();

    void prepare (double sampleRate, int /*samplesPerBlock*/);
    void loadSample (const juce::File& file);
    void renderNextBlock (juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi,
                           int startSample, int numSamples);
    void connectParameters (SamplerVoiceParams paramsToUse) { voiceParams = paramsToUse; }
    bool hasSampleLoaded() const { return sampleLoaded; }

private:
    juce::Synthesiser synth;
    juce::AudioFormatManager formatManager;
    SamplerVoiceParams voiceParams;
    bool sampleLoaded = false;
    static constexpr int numVoices = 8;
};
