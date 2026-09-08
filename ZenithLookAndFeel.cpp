#include "ZenithLookAndFeel.h"

namespace ZenithColours
{
    const juce::Colour background { 0xff12141a };
    const juce::Colour panel      { 0xff1a1d24 };
    const juce::Colour border     { 0xff2a2e38 };
    const juce::Colour teal       { 0xff3fd6c0 };
    const juce::Colour purple     { 0xffc084fc };
    const juce::Colour text       { 0xffe6e8ec };
    const juce::Colour textDim    { 0xff9aa0ab };
}

ZenithLookAndFeel::ZenithLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, ZenithColours::background);
    setColour (juce::Label::textColourId, ZenithColours::text);

    setColour (juce::Slider::textBoxTextColourId, ZenithColours::text);
    setColour (juce::Slider::textBoxBackgroundColourId, ZenithColours::panel);
    setColour (juce::Slider::textBoxOutlineColourId, ZenithColours::border);

    setColour (juce::TextButton::buttonColourId, ZenithColours::panel);
    setColour (juce::TextButton::textColourOffId, ZenithColours::text);
    setColour (juce::TextButton::textColourOnId, ZenithColours::teal);

    setColour (juce::ToggleButton::textColourId, ZenithColours::text);
    setColour (juce::ToggleButton::tickColourId, ZenithColours::teal);

    setColour (juce::ComboBox::backgroundColourId, ZenithColours::panel);
    setColour (juce::ComboBox::textColourId, ZenithColours::text);
    setColour (juce::ComboBox::outlineColourId, ZenithColours::border);
    setColour (juce::ComboBox::arrowColourId, ZenithColours::teal);

    setColour (juce::PopupMenu::backgroundColourId, ZenithColours::panel);
    setColour (juce::PopupMenu::textColourId, ZenithColours::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, ZenithColours::border);
    setColour (juce::PopupMenu::highlightedTextColourId, ZenithColours::teal);

    setColour (juce::TabbedComponent::backgroundColourId, ZenithColours::background);
    setColour (juce::TabbedButtonBar::tabOutlineColourId, ZenithColours::border);

    setColour (juce::AlertWindow::backgroundColourId, ZenithColours::panel);
    setColour (juce::AlertWindow::textColourId, ZenithColours::text);
}

void ZenithLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                           juce::Slider&)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (4.0f);
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centre = bounds.getCentre();
    auto lineWidth = juce::jmin (4.0f, radius * 0.22f);
    auto arcRadius = radius - lineWidth * 0.5f;

    // trilho de fundo
    juce::Path backgroundArc;
    backgroundArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                  rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (ZenithColours::border);
    g.strokePath (backgroundArc, juce::PathStrokeType (lineWidth, juce::PathStrokeType::curved,
                                                         juce::PathStrokeType::rounded));

    // arco de valor
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                             rotaryStartAngle, toAngle, true);
    g.setColour (ZenithColours::teal);
    g.strokePath (valueArc, juce::PathStrokeType (lineWidth, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));

    // disco central
    auto innerRadius = arcRadius - lineWidth;
    g.setColour (ZenithColours::panel);
    g.fillEllipse (centre.x - innerRadius, centre.y - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

    // ponteiro
    juce::Path pointer;
    auto pointerLength = innerRadius * 0.75f;
    auto pointerThickness = 2.2f;
    pointer.addRoundedRectangle (-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength, 1.0f);
    pointer.applyTransform (juce::AffineTransform::rotation (toAngle).translated (centre));
    g.setColour (ZenithColours::text);
    g.fillPath (pointer);
}

void ZenithLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                               const juce::Colour&, bool isMouseOverButton, bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);

    auto baseColour = ZenithColours::panel;
    if (button.getToggleState())
        baseColour = ZenithColours::border;
    if (isButtonDown)
        baseColour = ZenithColours::border;
    else if (isMouseOverButton)
        baseColour = baseColour.brighter (0.1f);

    g.setColour (baseColour);
    g.fillRoundedRectangle (bounds, 6.0f);

    g.setColour (button.getToggleState() ? ZenithColours::teal : ZenithColours::border);
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);
}

void ZenithLookAndFeel::drawTabButton (juce::TabBarButton& button, juce::Graphics& g,
                                        bool /*isMouseOver*/, bool /*isMouseDown*/)
{
    auto area = button.getLocalBounds();

    g.setColour (button.getToggleState() ? ZenithColours::panel : ZenithColours::background);
    g.fillRect (area);

    if (button.getToggleState())
    {
        g.setColour (ZenithColours::teal);
        g.fillRect (area.removeFromBottom (2));
    }

    g.setColour (button.getToggleState() ? ZenithColours::teal : ZenithColours::textDim);
    g.setFont (13.0f);
    g.drawFittedText (button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, 1);
}

int ZenithLookAndFeel::getTabButtonBestWidth (juce::TabBarButton& button, int)
{
    return juce::jmax (64, button.getButtonText().length() * 8 + 24);
}

void ZenithLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                       int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float> (0, 0, (float) width, (float) height).reduced (0.5f);

    g.setColour (ZenithColours::panel);
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (ZenithColours::border);
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

    juce::Path arrow;
    auto arrowZone = juce::Rectangle<float> ((float) width - 20.0f, 0.0f, 16.0f, (float) height);
    arrow.addTriangle (arrowZone.getX(), arrowZone.getCentreY() - 3.0f,
                        arrowZone.getRight(), arrowZone.getCentreY() - 3.0f,
                        arrowZone.getCentreX(), arrowZone.getCentreY() + 4.0f);
    g.setColour (box.isEnabled() ? ZenithColours::teal : ZenithColours::textDim);
    g.fillPath (arrow);
}
