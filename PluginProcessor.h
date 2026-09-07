#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

/**
    FASE 1 — Plugin mínimo.
    Objetivo desta fase: ter um plugin de instrumento que compila, carrega no DAW,
    aceita MIDI e não produz crash nem ruído indevido. Nenhum DSP real ainda —
    isso entra na FASE 2 (SamplerEngine).
*/
class ZenithGranularAudioProcessor : public juce::AudioProcessor
{
public:
    ZenithGranularAudioProcessor();
    ~ZenithGranularAudioProcessor() override;

    // --- ciclo de vida do áudio ---
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // --- editor (UI) ---
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    // --- identidade do plugin ---
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    // --- programas/presets (ainda não usado na v1 mínima) ---
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    // --- estado (save/load) ---
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    // Placeholder: na FASE 2 isto vira o SamplerEngine (carregamento de sample,
    // ADSR, pitch). Por agora existe só para provar que o plugin recebe MIDI
    // e devolve um bloco de áudio válido (silêncio) sem crashar.
    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZenithGranularAudioProcessor)
};
