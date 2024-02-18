#pragma once

#include "BirdHouseLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>

class ActivityIndicator : public juce::Component
{
public:
    ActivityIndicator() : color (BirdHouse::Colours::bg), outlineColor (BirdHouse::Colours::bg)
    {
    }

    void setColour (const juce::Colour& newColor)
    {
        color = newColor;
        repaint(); // Trigger a repaint to update the color
    }

    void setOutlineColour (const juce::Colour& newColor)
    {
        outlineColor = newColor;
        repaint(); // Trigger a repaint to update the color
    }

    void setOutlineThickness (float newThickness)
    {
        lineThickness = newThickness;
        repaint(); // Trigger a repaint to update the thickness
    }

    const juce::Colour& getColour() const
    {
        return color;
    }

protected:
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Fill the square with the color and draw an outline
        g.setColour (color);
        g.fillRoundedRectangle (bounds, 5.0f);
        g.setColour (outlineColor);
        g.drawRect (bounds, lineThickness);
    }

private:
    juce::Colour color, outlineColor; // The color of the square
    float lineThickness = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ActivityIndicator)
};
