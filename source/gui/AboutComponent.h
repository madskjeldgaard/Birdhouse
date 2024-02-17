#pragma once

#include "BirdHouseLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>

class AboutComponent : public juce::Component
{
public:
    AboutComponent()
    {
        // Set up the components with their properties
        headlineLabel.setText ("BirdHouse. v" VERSION, juce::dontSendNotification);
        headlineLabel.setFont (juce::Font (36.0f, juce::Font::bold));
        headlineLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (headlineLabel);

        descriptionLabel.setText ("An OSC to MIDI bridge written by Mads Kjeldgaard.", juce::dontSendNotification);
        descriptionLabel.setFont (juce::Font (16.0f, juce::Font::italic));
        descriptionLabel.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (descriptionLabel);

        hyperlinkButton.setButtonText ("Find out more.");
        hyperlinkButton.setURL (juce::URL ("https://github.com/madskjeldgaard/BirdHouse"));
        addAndMakeVisible (hyperlinkButton);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (10);
        headlineLabel.setBounds (area.removeFromTop (40));
        descriptionLabel.setBounds (area.removeFromTop (60));
        hyperlinkButton.setBounds (area.removeFromTop (30).reduced (area.getWidth() / 4, 0));
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (BirdHouse::Colours::bgDark);
    }

private:
    juce::Label headlineLabel;
    juce::Label descriptionLabel;
    juce::HyperlinkButton hyperlinkButton { "Visit GitHub", juce::URL ("https://github.com/madskjeldgaard/BirdHouse") };
};

static void showAboutWindow()
{
    // Create the AboutComponent on the heap
    auto* aboutComponent = new AboutComponent();
    aboutComponent->setSize (400, 200);

    juce::DialogWindow::LaunchOptions options;

    // Set the content component, which will be deleted automatically by the DialogWindow
    options.content.setOwned (aboutComponent);

    // Configure the rest of the options
    options.dialogTitle = "About BirdHouse";
    options.componentToCentreAround = nullptr; // Center on screen if no component provided
    options.dialogBackgroundColour = juce::Colours::lightgrey;
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = false;
    options.resizable = false;

    // Launch the dialog window asynchronously
    options.launchAsync();
}
