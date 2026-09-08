#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
    FASE 7 (início) — identidade visual própria: knobs circulares com arco
    teal, fundo escuro, abas e botões consistentes com o conceito de UI
    desenhado no início do projeto. Isto substitui o "look" genérico do JUCE
    por defeito, sem ainda ser o design 100% definitivo (waveform animada com
    grãos, etc. continuam a evoluir nas próximas fases).
*/
class ZenithLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ZenithLookAndFeel();

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                            float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                            juce::Slider& slider) override;

    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                bool isMouseOverButton, bool isButtonDown) override;

    void drawTabButton (juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown) override;

    int getTabButtonBestWidth (juce::TabBarButton& button, int tabDepth) override;

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                        int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;
};
