#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <array>
#include <atomic>
#include <cmath>

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

/**
    Estado partilhado, lido pela UI (~30x/s) para desenhar os grãos por cima
    da waveform. Cada voz escreve aqui a posição (0-1, fração do buffer) dos
    seus grãos ativos; -1 significa "sem grão nesse slot". Só leituras e
    escritas atómicas — nada de locks entre a audio thread e a UI thread.
*/
struct GrainVisualizer
{
    static constexpr int grainsPerVoice = 3;
    static constexpr int maxVoices = 8;
    static constexpr int maxVisibleGrains = grainsPerVoice * maxVoices;

    std::array<std::atomic<float>, maxVisibleGrains> positions;

    GrainVisualizer() { for (auto& p : positions) p.store (-1.0f); }
};

/**
    Tabela pré-calculada da janela Hann (1024 pontos), partilhada por todas
    as vozes. Evita chamar std::cos() em cada amostra de cada grão — com
    até 16 grãos por voz × 8 vozes, isso seria até 128 chamadas de cosseno
    por amostra de áudio. Uma tabela + interpolação linear é muito mais leve.
*/
class HannWindowTable
{
public:
    static constexpr int size = 1024;

    HannWindowTable()
    {
        for (int i = 0; i < size; ++i)
        {
            const auto t = (float) i / (float) (size - 1);
            table[(size_t) i] = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * t);
        }
    }

    /** t deve estar entre 0.0 e 1.0. */
    float getValue (float t) const
    {
        const auto pos = juce::jlimit (0.0f, (float) (size - 1), t * (float) (size - 1));
        const auto i0 = (int) pos;
        const auto i1 = juce::jmin (i0 + 1, size - 1);
        const auto frac = pos - (float) i0;
        return table[(size_t) i0] + frac * (table[(size_t) i1] - table[(size_t) i0]);
    }

private:
    std::array<float, size> table;
};

inline const HannWindowTable& getHannWindowTable()
{
    static const HannWindowTable table;
    return table;
}

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
    explicit ZenithSamplerVoice (const SamplerVoiceParams& engineParams, GrainVisualizer& visualizerToUse, int slotBaseToUse)
        : params (engineParams), visualizer (visualizerToUse), slotBase (slotBaseToUse)
    {
    }

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
            clearVisualization();
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
                clearVisualization();
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
                                ? juce::jlimit (0.0f, 1.0f, params.granularMix->load (std::memory_order_relaxed) / 100.0f)
                                : 0.0f;

            float grainL = 0.0f, grainR = 0.0f;
            if (mix > 0.0001f)
            {
                advanceGrains (data, numSourceChannels, numSourceSamples, grainL, grainR);
                grainsActiveThisBlock = true;
            }

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

        if (grainsActiveThisBlock)
        {
            publishVisualization (numSourceSamples);
            grainsActiveThisBlock = false;
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
        const auto density = params.grainDensity != nullptr ? juce::jmax (0.1f, params.grainDensity->load (std::memory_order_relaxed)) : 20.0f;

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
            const auto window = getHannWindowTable().getValue (t);

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

    /**
        Publica as posições dos primeiros grãos ativos para a UI desenhar
        (WaveformDisplay lê isto a ~30x/s). Chamado só UMA VEZ por bloco de
        áudio (não por amostra) — a UI não precisa de mais resolução que
        isso, e cada chamada evitada poupa até 3 escritas atómicas por voz.
    */
    void publishVisualization (int numSourceSamples)
    {
        int shown = 0;
        for (auto& g : grains)
        {
            if (shown >= GrainVisualizer::grainsPerVoice)
                break;
            if (g.active)
            {
                const auto norm = (float) (g.readPos / (double) juce::jmax (1, numSourceSamples - 1));
                visualizer.positions[slotBase + shown].store (juce::jlimit (0.0f, 1.0f, norm));
                ++shown;
            }
        }
        for (; shown < GrainVisualizer::grainsPerVoice; ++shown)
            visualizer.positions[slotBase + shown].store (-1.0f);
    }

    void clearVisualization()
    {
        for (int i = 0; i < GrainVisualizer::grainsPerVoice; ++i)
            visualizer.positions[slotBase + i].store (-1.0f);
    }

    static constexpr int maxGrains = 16;
    static constexpr float grainGain = 0.6f; // atenuação para evitar acumulação de volume com muitos grãos

    const SamplerVoiceParams& params;
    GrainVisualizer& visualizer;
    int slotBase;
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
    bool grainsActiveThisBlock = false;
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

    /** Cópia segura (thread-safe) das posições dos grãos ativos, para a UI desenhar. */
    std::array<float, GrainVisualizer::maxVisibleGrains> getGrainSnapshot() const
    {
        std::array<float, GrainVisualizer::maxVisibleGrains> out;
        for (int i = 0; i < GrainVisualizer::maxVisibleGrains; ++i)
            out[(size_t) i] = grainVisualizer.positions[(size_t) i].load();
        return out;
    }

private:
    juce::Synthesiser synth;
    juce::AudioFormatManager formatManager;
    SamplerVoiceParams voiceParams;
    GrainVisualizer grainVisualizer;
    bool sampleLoaded = false;
    static constexpr int numVoices = GrainVisualizer::maxVoices;
};
