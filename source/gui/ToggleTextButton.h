#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace juce {
class ToggleTextButton : public juce::TextButton {
 public:
  explicit ToggleTextButton(const juce::String &buttonText) : TextButton(buttonText) {
    // Set the button to trigger click events on mouse down and up
    // to better mimic a toggle button's behavior.
    setClickingTogglesState(true);
  }

  void paintButton(juce::Graphics &g, bool shouldDrawButtonAsHighlighted,
                   bool shouldDrawButtonAsDown) override {
    // Optionally customize the appearance based on the toggle state
    if (getToggleState()) {
      auto color = findColour(juce::TextButton::buttonOnColourId);
      g.fillAll(color);  // For example, fill with light blue when toggled on
    } else {
      auto color = findColour(juce::TextButton::buttonColourId);
      g.fillAll(color);  // Default or off state
    }

    // Call the superclass method to handle text drawing and other standard behaviors
    TextButton::paintButton(g, shouldDrawButtonAsHighlighted,
                            shouldDrawButtonAsDown);
  }
};
