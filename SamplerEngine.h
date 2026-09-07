#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <array>
#include <atomic>

/**
    Ponteiros para os valores ao vivo da APVTS. Cada voz consulta isto a cada
    amostra, para reagir em tempo real aos knobs da UI sem recriar nada.
*/
struct SamplerVoiceParams
{
    // Sampler (Fase 2)
    std::atomic<float>* attack   = nullptr; // ms
    std::atomic<float>* decay    = nullptr; // ms
    std::atomic<float>* sustain  = nullptr; // %
    std::atomic<float>* release  = nullptr; // ms
    std::atomic<float>* pitch    = nullptr; // semitons
    std::atomic<float>* fineTune = nullptr; // cents

    // Granular (Fase 3)
    std::atomic<float>* grainSize            = nullptr; // ms
    std::atomic<float>* grainDensity         = nullptr; // grãos/s
    std::atomic<float>* grainPosition        = nullptr; // % do buffer
    std::atomic<float>* grainPositionRandom  = nullptr; // %
    std::atomic<float>* grainPitch           = nullptr; // semitons
    std::atomic<float>* grainPitchRandom     = nullptr; // % (espalhamento aleatório)
    std::atomic<float>* grainPan             = nullptr; // % (dispersão estéreo)
    std::atomic<float>* granularMix          = nullptr; // % — 0 = só sampler normal, 100 = só textura granular
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
    Uma voz do sampler/granular: mistura duas fontes —
      1) leitura linear normal do sample (comportamento da Fase 2);
      2) uma nuvem de grãos lidos do mesmo buffer, com posição, tamanho,
         densidade e pitch próprios (Fase 3).
    O knob "granularMix" (0-100%) faz o crossfade entre as duas.
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
        noteSemitoneOffset = (double) (midiNoteNumber - currentSound->getRootNote());

        const auto extraPitch    = params.pitch    != nullptr ? (double) params.pitch->load()    : 0.0;
        const auto fineTuneCents = params.fineTune != nullptr ? (double) params.fineTune->load()  : 0.0;

        pitchRatio = std::pow (2.0, (noteSemitoneOffset + extraPitch + fineTuneCents / 100.0) / 12.0)
                     * (currentSound->getSourceSampleRate() / getSampleRate());

        juce::ADSR::Parameters adsrParams;
        adsrParams.attack  = params.attack  != nullptr ? juce::jmax (0.001f, params.attack->load()  / 1000.0f) : 0.01f;
        adsrParams.decay   = params.decay   != nullptr ? juce::jmax (0.001f, params.decay->load()   / 1000.0f) : 0.1f;
        adsrParams.sustain = params.sustain != nullptr ? params.sustain->load() / 100.0f : 1.0f;
        adsrParams.release = params.release != nullptr ? juce::jmax (0.001f, params.release->load() / 1000.0f) : 0.2f;
        adsr.setSampleRate (getSampleRate());
        adsr.setParameters (adsrParams);
        adsr.noteOn();

        for (auto& g : grains)
            g.active = false;
        samplesUntilNextGrain = 0.0;
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
            if (! adsr.isActive())
            {
                clearCurrentNote();
                currentSound = nullptr;
                break;
            }

            const auto env = adsr.getNextSample();

            const bool dryValid = sourcePosition < (double) (numSourceSamples - 1);
            int dryPos = 0;
            float dryFrac = 0.0f;
            if (dryValid)
            {
                dryPos  = (int) sourcePosition;
                dryFrac = (float) (sourcePosition - (double) dryPos);
            }

            const auto mix = params.granularMix != nullptr
                                ? juce::jlimit (0.0f, 1.0f, params.granularMix->load() / 100.0f)
                                : 0.0f;

            float grainL = 0.0f, grainR = 0.0f;
            if (mix > 0.0001f)
                advanceGrains (data, numSourceChannels, numSourceSamples, grainL, grainR);

            for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
            {
                float dry = 0.0f;
                if (dryValid)
                {
                    const auto srcChannel = juce::jmin (ch, numSourceChannels - 1);
                    const auto s0 = data.getSample (srcChannel, dryPos);
                    const auto s1 = data.getSample (srcChannel, juce::jmin (dryPos + 1, numSourceSamples - 1));
                    dry = s0 + dryFrac * (s1 - s0);
                }

                const auto gran = (ch == 0) ? grainL : grainR;
                const auto mixed = dry * (1.0f - mix) + gran * mix;
                outputBuffer.addSample (ch, startSample + i, mixed * env * currentVelocity);
            }

            if (dryValid)
                sourcePosition += pitchRatio;
        }
    }

private:
    struct Grain
    {
        bool active = false;
        double readPos = 0.0;
        double pitchRatio = 1.0;
        int ageSamples = 0;
        int lengthSamples = 1;
        float pan = 0.5f; // 0 = esquerda, 1 = direita
    };

    void spawnGrain (int numSourceSamples, double sourceSampleRate)
    {
        for (auto& g : grains)
        {
            if (g.active)
                continue;

            const auto posFraction    = params.grainPosition       != nullptr ? juce::jlimit (0.0f, 1.0f, params.grainPosition->load() / 100.0f) : 0.0f;
            const auto posRandomAmt   = params.grainPositionRandom != nullptr ? juce::jlimit (0.0f, 1.0f, params.grainPositionRandom->load() / 100.0f) : 0.0f;
            const auto jitter         = (random.nextFloat() * 2.0f - 1.0f) * posRandomAmt;
            const auto startFraction  = juce::jlimit (0.0f, 1.0f, posFraction + jitter);

            const auto grainSizeMs    = params.grainSize != nullptr ? juce::jmax (1.0f, params.grainSize->load()) : 80.0f;
            const auto grainPitchSemi = params.grainPitch != nullptr ? (double) params.grainPitch->load() : 0.0;
            const auto pitchRandomAmt = params.grainPitchRandom != nullptr ? juce::jlimit (0.0f, 1.0f, params.grainPitchRandom->load() / 100.0f) : 0.0f;
            const auto pitchRandom    = (random.nextFloat() * 2.0f - 1.0f) * pitchRandomAmt * 12.0f; // até +-12 st no máximo

            const auto panSpread = params.grainPan != nullptr ? juce::jlimit (0.0f, 1.0f, params.grainPan->load() / 100.0f) : 0.5f;

            g.active        = true;
            g.readPos       = (double) startFraction * (double) (numSourceSamples - 1);
            g.pitchRatio    = pitchRatio * std::pow (2.0, (grainPitchSemi + pitchRandom) / 12.0);
            g.ageSamples    = 0;
            g.lengthSamples = juce::jmax (1, (int) (grainSizeMs / 1000.0 * getSampleRate()));
            g.pan           = 0.5f + (random.nextFloat() * 2.0f - 1.0f) * 0.5f * panSpread;

            juce::ignoreUnused (sourceSampleRate);
            return;
        }
        // sem slot livre — este grão é simplesmente ignorado (não há alocação dinâmica)
    }

    void advanceGrains (const juce::AudioBuffer<float>& data, int numSourceChannels, int numSourceSamples,
                         float& outL, float& outR)
    {
        const auto density = params.grainDensity != nullptr ? juce::jmax (0.1f, params.grainDensity->load()) : 20.0f;

        samplesUntilNextGrain -= 1.0;
        if (samplesUntilNextGrain <= 0.0)
        {
            spawnGrain (numSourceSamples, currentSound->getSourceSampleRate());
            samplesUntilNextGrain = getSampleRate() / (double) density;
        }

        for (auto& g : grains)
        {
            if (! g.active)
                continue;

            const auto t = (float) g.ageSamples / (float) g.lengthSamples;
            const auto window = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * t); // janela Hann

            const auto pos  = (int) g.readPos;
            const auto frac = (float) (g.readPos - (double) pos);

            if (pos >= 0 && pos < numSourceSamples - 1)
            {
                const auto srcChannel = 0; // simplificação: lê do 1º canal do sample para o texturizador
                const auto s0 = data.getSample (juce::jmin (srcChannel, numSourceChannels - 1), pos);
                const auto s1 = data.getSample (juce::jmin (srcChannel, numSourceChannels - 1), pos + 1);
                const auto sample = (s0 + frac * (s1 - s0)) * window * grainGain;

                outL += sample * (1.0f - g.pan);
                outR += sample * g.pan;
            }

            g.readPos += g.pitchRatio;
            ++g.ageSamples;

            if (g.ageSamples >= g.lengthSamples || g.readPos < 0.0 || g.readPos >= (double) (numSourceSamples - 1))
                g.active = false;
        }
    }

    static constexpr int maxGrains = 16;
    static constexpr float grainGain = 0.6f; // atenuação para evitar acumulação de volume com muitos grãos

    const SamplerVoiceParams& params;
    ZenithSamplerSound* currentSound = nullptr;

    // leitura linear (Fase 2)
    double sourcePosition = 0.0;
    double pitchRatio = 1.0;
    double noteSemitoneOffset = 0.0;
    float currentVelocity = 1.0f;
    juce::ADSR adsr;

    // nuvem de grãos (Fase 3)
    std::array<Grain, maxGrains> grains;
    double samplesUntilNextGrain = 0.0;
    juce::Random random;
};

/** Wrapper de alto nível: carrega um ficheiro de áudio e expõe render/prepare ao PluginProcessor. */
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
