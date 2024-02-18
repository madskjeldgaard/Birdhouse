#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);

    // Set default look and feel
    lookAndFeel = std::make_unique<BirdHouse::BirdHouseLookAndFeel>();
    juce::LookAndFeel::setDefaultLookAndFeel (lookAndFeel.get());

    auto defaultFontSize = 24.0f;

    // Link
    hyperlinkButton.setButtonText ("(?)");
    hyperlinkButton.setFont (juce::Font (defaultFontSize, juce::Font::plain), false);
    hyperlinkButton.setColour (juce::HyperlinkButton::textColourId, BirdHouse::Colours::magenta);
    hyperlinkButton.setJustificationType (juce::Justification::right);
    addAndMakeVisible (hyperlinkButton);

    // Labels
    auto labelFont = juce::Font (defaultFontSize, juce::Font::bold);
    auto labelColour = BirdHouse::Colours::blue;
    oscBridgeChannelLabels = std::make_unique<OSCBridgeChannelLabels>();
    oscBridgeChannelLabels->setFont (labelFont);
    oscBridgeChannelLabels->setColour (labelColour);
    addAndMakeVisible (*oscBridgeChannelLabels);

    // Set font for title
    auto titleFont = juce::Font (defaultFontSize * 1.33f, juce::Font::bold);
    titleLabel.setFont (titleFont);
    titleLabel.setText ("BirdHouse. An OSC to MIDI bridge. v" VERSION ".", juce::dontSendNotification);
    titleLabel.setColour (juce::Label::textColourId, BirdHouse::Colours::fg);
    titleLabel.setVisible (true);
    addAndMakeVisible (titleLabel);

    // Get state
    auto state = processorRef.oscBridgeState;

    // Set up each channel
    auto numChannels = 8;
    for (auto i = 0; i < numChannels; ++i)
    {
        auto channelID = juce::Identifier (juce::String (i));
        auto channelState = state.getChildWithName ("ChannelSettings").getChild (i);

        oscBridgeChannelEditors.push_back (std::make_unique<OSCBridgeChannelEditor>());

        // Set default values for the editors
        oscBridgeChannelEditors.back()->setFont (juce::Font (defaultFontSize, juce::Font::plain));
        if (channelState.isValid())
        {
            oscBridgeChannelEditors.back()->setState (channelState);
        }
        else
        {
            juce::Logger::writeToLog ("Channel state is invalid");
        }

        addAndMakeVisible (*oscBridgeChannelEditors.back());
    }

    auto globalState = state.getChildWithName ("GlobalSettings");

    // Port
    auto port = globalState.getProperty ("Port", 8000);
    portEditor.setFont (juce::Font (defaultFontSize, juce::Font::plain));
    portEditor.setJustification (juce::Justification::left);
    portEditor.setText (juce::String (port));
    portEditor.setInputRestrictions (5, "1234567890");

    // Filter text
    portEditor.onTextChange = [this] {
        auto newport = portEditor.getText().getIntValue();
        auto globalSettings = processorRef.oscBridgeState.getChildWithName ("GlobalSettings");

        // Port can only be a number from 0 to 65535
        // If the port is not a number, set it to 8000
        // If the port is less than 0, set it to 0
        // If the port is greater than 65535, set it to 65535
        // Otherwise, set it to the new value
        if (newport < 0)
        {
            portEditor.setText ("0", false);
        }
        else if (newport > 65535)
        {
            portEditor.setText ("65535", false);
        }
    };

    // On return -> lose focus
    portEditor.onReturnKey = [this] {
        portEditor.focusLost (FocusChangeType::focusChangedDirectly);
    };

    // Focus lost -> update state
    portEditor.onFocusLost = [this] {
        auto newport = portEditor.getText().getIntValue();
        auto globalSettings = processorRef.oscBridgeState.getChildWithName ("GlobalSettings");
        globalSettings.setProperty ("Port", newport, nullptr);
    };

    addAndMakeVisible (portEditor);

    // Port label
    portLabel.setFont (labelFont);
    portLabel.setColour (juce::Label::textColourId, labelColour);
    portLabel.setText ("Port", juce::dontSendNotification);
    addAndMakeVisible (portLabel);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1000, 500);

    // Allow resizing
    setResizable (true, false);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    auto connectionStatus = processorRef.oscBridgeState.getChildWithName ("GlobalSettings").getProperty ("ConnectionStatus", false);

    if (connectionStatus)
    {
        portEditor.applyColourToAllText (BirdHouse::Colours::green, true);
    }
    else
    {
        portEditor.applyColourToAllText (BirdHouse::Colours::red, true);
    }

    // const auto timeThresholdMS = 100;
    // Iterate over all channels and see if it's been more than tan the timeThresholdMS since last message
    // If it has, set the path label colour to BirdHouse::Colour::fg
    // If it hasn't, set the path label colour to BirdHouse::Colour::magenta

    // auto index = 0u;
    // for (auto& oscBridgeChannelEditor : oscBridgeChannelEditors)
    // {
    //     auto timeSinceLastValue = processorRef.getTimeSinceLastValueChannel (index);

    //     // Paint an active color if the channel has been active recently
    //     // if (timeSinceLastValue < timeThresholdMS)
    //     // {
    //     //     juce::Logger::writeToLog ("timeSinceLastMessage: " + juce::String (timeSinceLastValue));
    //         auto amountActivity = processorRef.getNormalizedValueFromChannel (index);
    //         oscBridgeChannelEditor->setActivityAmount (amountActivity);
    //     // }
    //     // else
    //     // {
    //     //     // Else normal color
    //     //     oscBridgeChannelEditor->setActivityAmount (-1.f);
    //     // }

    //     index++;
    // }
}

void PluginEditor::resized()
{
    auto area = getLocalBounds();
    auto totalChannels = oscBridgeChannelEditors.size() + 4; // Including title, port, and hyperlink in count
    auto itemHeight = area.getHeight() / totalChannels;

    // Title area at the top
    titleLabel.setBounds (area.removeFromTop (itemHeight));

    // Adjusting for the channels' labels and editors
    oscBridgeChannelLabels->setBounds (area.removeFromTop (itemHeight));

    for (auto& oscBridgeChannelEditor : oscBridgeChannelEditors)
    {
        oscBridgeChannelEditor->setBounds (area.removeFromTop (itemHeight));
    }

    // Layout for portLabel, portEditor, and hyperlinkButton at the bottom
    auto bottomArea = area.removeFromBottom (itemHeight); // Reserve space at the bottom

    // Split the bottom area into three parts
    auto portLabelWidth = bottomArea.getWidth() * 0.1f; // Adjust the ratio as needed
    auto portEditorWidth = bottomArea.getWidth() * (1.0f / 7.0f); // 1/7th of the width
    auto hyperlinkWidth = bottomArea.getWidth() * 0.2f; // Adjust as needed

    // Place portLabel on the left side of the bottom area
    portLabel.setBounds (bottomArea.removeFromLeft (portLabelWidth));

    // Place portEditor next to portLabel
    portEditor.setBounds (bottomArea.removeFromLeft (portEditorWidth));

    // Place hyperlinkButton on the far right of the bottom area
    hyperlinkButton.setBounds (bottomArea.removeFromRight (hyperlinkWidth));
}
