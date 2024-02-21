#pragma once

#include "BirdHouseLookAndFeel.h"
#include <deque>
#include <juce_gui_basics/juce_gui_basics.h>

template <size_t MaxValues = 64>
class ActivityIndicator : public juce::Component, juce::Timer
{
public:
    ActivityIndicator()
        : maxValueCount (MaxValues)
    {
        values.assign (maxValueCount, 0.0f);
        setBackgroundColour (BirdHouse::Colours::bgDark);
        setOutlineColour (findColour(juce::TextEditor::outlineColourId));
        setValueColour (BirdHouse::Colours::blue);

        startTimerHz (30); // Start the timer with a frequency of 30 Hz
    }

    // Make sure it alwyays repaints, even when the window is out of focus
    void timerCallback() override
    {
        repaint();
    }

    /**
     * @brief Add new value, and remove the oldest value.
     *
     * @param newValue The new value to add. Must be between 0.0f and 1.0f.
     */
    void addValue (float newValue)
    {
        // Normalize and clamp newValue between 0.0f and 1.0f
        newValue = std::max (0.0f, std::min (newValue, 1.0f));
        values.pop_front();
        values.push_back (newValue);
        repaint(); // Request a component repaint to reflect the new value
    }

    void paint (juce::Graphics& g) override
    {
        auto outlineSize = 1.0f;
        auto bounds = getLocalBounds().toFloat();
        auto widthPerValue = bounds.getWidth() / maxValueCount;

        // Draw background
        g.fillAll (backgroundColour);

        // Draw each value
        for (size_t i = 0; i < values.size(); ++i)
        {
            float valueHeight = values[i] * bounds.getHeight();
            auto valueX = i * widthPerValue;
            auto valueY = bounds.getBottom() - valueHeight;
            juce::Rectangle<float> valueRect (valueX, valueY, widthPerValue, valueHeight);

            // juce::ColourGradient colorGradient (BirdHouse::Colours::magenta, valueRect.getX(), valueRect.getCentreY(), BirdHouse::Colours::yellow, valueRect.getRight(), valueRect.getCentreY(), false);
            g.setColour (valueColour);
            // g.setGradientFill (colorGradient);
            g.fillRect (valueRect);
        }

        // Draw outline
        g.setColour (outlineColour);
        g.drawRect (bounds, outlineSize); // 1.0f is the line thickness
    }

    // Setter methods for customizing appearance
    void setBackgroundColour (juce::Colour colour) { backgroundColour = colour; }
    void setOutlineColour (juce::Colour colour) { outlineColour = colour; }
    void setValueColour (juce::Colour colour) { valueColour = colour; }

private:
    std::deque<float> values { MaxValues, 0.f }; // Fixed-size deque to store the last N values
    size_t maxValueCount; // The maximum number of values to display

    // Appearance
    juce::Colour backgroundColour = juce::Colours::black;
    juce::Colour outlineColour = juce::Colours::white;
    juce::Colour valueColour = juce::Colours::green;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ActivityIndicator)
};
