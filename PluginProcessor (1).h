#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "SamplerEngine.h"

/**
    FASE 2 — SamplerEngine ligado. O plugin agora carrega um .wav (via botão
    na UI) e toca-o por nota MIDI, com ADSR e pitch controláveis em tempo real
    pelos parâmetros da APVTS (para depois seres automatizável no DAW).
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

    // --- API usada pela UI ---
    void loadSample (const juce::File& file);
    bool hasSampleLoaded() const { return samplerEngine.hasSampleLoaded(); }

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    SamplerEngine samplerEngine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZenithGranularAudioProcessor)
};
