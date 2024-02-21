#pragma once

#include "../bridge/OSCBridgeChannel.h"
#include "ActivityIndicator.h"
#include "OSCBridgeChannelLabels.h"
#include <juce_gui_basics/juce_gui_basics.h>

// #include "ToggleTextButton.h"

class OSCBridgeChannelEditor : public juce::Component
{
public:
    OSCBridgeChannelEditor()
    {
        // Setup components
        // addAndMakeVisible (activityIndicator);

        addAndMakeVisible (pathEditor);
        // onTextChange->Filter text
        pathEditor.onTextChange = [this] {
            // Filter the characters
            // Must start with the "/" character
            // Must not contain any spaces
            // Has to be more than 1 character

            auto string = pathEditor.getText();

            if (string.isNotEmpty())
            {
                if (string[0] != '/')
                {
                    string = "/" + string;
                }

                if (string.contains (" "))
                {
                    string = string.replace (" ", "_");
                }

                pathEditor.setText (string, false);
            }
        };

        // Return -> focus lost
        pathEditor.onReturnKey = [&] {
            pathEditor.focusLost (FocusChangeType::focusChangedDirectly);
        };

        // Focus lost -> update state
        pathEditor.onFocusLost = [&] {
            const auto whatChanged = juce::Identifier ("Path");
            updateChannelSettings (whatChanged);
        };

        addAndMakeVisible (inputMinEditor);
        inputMinEditor.setInputRestrictions (5, "0123456789.-");

        // text change-> filter text
        inputMinEditor.onTextChange = [&] {
        };

        // Return -> focus lost
        inputMinEditor.onReturnKey = [&] {
            inputMinEditor.focusLost (FocusChangeType::focusChangedDirectly);
        };

        // Focus lost -> update state
        inputMinEditor.onFocusLost = [&] {
            const auto whatChanged = juce::Identifier ("InputMin");
            updateChannelSettings (whatChanged);
        };

        addAndMakeVisible (inputMaxEditor);
        inputMaxEditor.setInputRestrictions (5, "0123456789.-");
        // Filter text
        inputMaxEditor.onTextChange = [&] {

        };

        // Return -> focus lost
        inputMaxEditor.onReturnKey = [&] {
            inputMaxEditor.focusLost (FocusChangeType::focusChangedDirectly);
        };

        // Focus lost -> update state
        inputMaxEditor.onFocusLost = [&] {
            const auto whatChanged = juce::Identifier ("InputMax");
            updateChannelSettings (whatChanged);
        };

        addAndMakeVisible (outputMidiChannelEditor);
        outputMidiChannelEditor.setInputRestrictions (2, "0123456789");

        // Filter text
        outputMidiChannelEditor.onTextChange = [this] {
            // Clip to midi range 1-16 for channels
            auto value = outputMidiChannelEditor.getText().getIntValue();

            if (value < 1)
            {
                value = 1;
            }
            else if (value > 16)
            {
                value = 16;
            }

            outputMidiChannelEditor.setText (juce::String (value), false);
        };

        // Return -> focus lost
        outputMidiChannelEditor.onReturnKey = [&] {
            outputMidiChannelEditor.focusLost (FocusChangeType::focusChangedDirectly);
        };

        // Focus lost -> update state
        outputMidiChannelEditor.onFocusLost = [&] {
            const auto whatChanged = juce::Identifier ("OutputMidiChannel");
            updateChannelSettings (whatChanged);
        };

        addAndMakeVisible (outputNumEditor);
        outputNumEditor.setInputRestrictions (3, "0123456789");

        // Filter text
        outputNumEditor.onTextChange = [&] {
            // Clip to midi range
            auto value = outputNumEditor.getText().getIntValue();

            // Clip to midi range
            if (value < 0)
            {
                value = 0;
            }
            else if (value > 127)
            {
                value = 127;
            }

            outputNumEditor.setText (juce::String (value), false);
        };

        // Return -> focus lost
        outputNumEditor.onReturnKey = [&] {
            outputNumEditor.focusLost (FocusChangeType::focusChangedDirectly);
        };

        // Focus lost -> update state
        outputNumEditor.onFocusLost = [&] {
            const auto whatChanged = juce::Identifier ("OutputMidiNum");
            updateChannelSettings (whatChanged);
        };

        outputMsgTypeComboBox.setColour (juce::ComboBox::arrowColourId, BirdHouse::Colours::fg);

        outputMsgTypeComboBox.addItem ("Note", birdhouse::MsgType::MidiNote + 1);
        outputMsgTypeComboBox.addItem ("CC", birdhouse::MsgType::MidiCC + 1);
        // outputMsgTypeComboBox.addItem ("Bend", OSCBridgeChannel::MidiBend + 1);
        addAndMakeVisible (outputMsgTypeComboBox);
        outputMsgTypeComboBox.onChange = [&] {
            const auto whatChanged = juce::Identifier ("MsgType");
            updateChannelSettings (whatChanged);
        };

        outputMsgTypeComboBox.setSelectedId (birdhouse::MsgType::MidiNote + 1);

        addAndMakeVisible (muteButton);
        muteButton.setToggleable (true);
        muteButton.setClickingTogglesState (true);
        muteButton.setButtonText ("Mute");
        muteButton.setColour (juce::TextButton::textColourOffId, BirdHouse::Colours::fg);
        muteButton.setColour (juce::TextButton::textColourOnId, BirdHouse::Colours::bg);
        muteButton.setColour (juce::TextButton::buttonColourId, BirdHouse::Colours::bg);
        muteButton.setColour (juce::TextButton::buttonOnColourId, BirdHouse::Colours::red);
        // Set up toggle button callback
        muteButton.onClick = [&] {
            const auto whatChanged = juce::Identifier ("Muted");
            updateChannelSettings (whatChanged);
        };

        // Set text justification
        auto justification = juce::Justification::left;
        setJustification (justification);

        // ActivityIndicator
        addAndMakeVisible (activityIndicator);

        addAndMakeVisible (titleLabel);
        titleLabel.setColour (juce::Label::ColourIds::textColourId, BirdHouse::Colours::blue);
        titleLabel.setJustificationType (juce::Justification::centred);
    }

    void setTextColour (juce::Colour colour)
    {
        const auto colourID = juce::Label::textColourId;
        inputMinEditor.setColour (colourID, colour);
        inputMaxEditor.setColour (colourID, colour);
        outputMidiChannelEditor.setColour (colourID, colour);
        outputNumEditor.setColour (colourID, colour);
        // titleLabel.setColour (colourID, colour);
        // muteButton.setColour (juce::TextButton::buttonColourId, colour);

        auto comboBoxID = juce::ComboBox::textColourId;
        outputMsgTypeComboBox.setColour (comboBoxID, colour);
    }


    void setFont (const juce::Font& font)
    {
        pathEditor.setFont (font);
        inputMinEditor.setFont (font);
        inputMaxEditor.setFont (font);
        outputMidiChannelEditor.setFont (font);
        outputNumEditor.setFont (font);
        titleLabel.setFont (font.withTypefaceStyle ("Bold")); // Bold
        // muteButton.setToggleable (font);
    }

    void setJustification (juce::Justification justification)
    {
        pathEditor.setJustification (justification);
        inputMinEditor.setJustification (justification);
        inputMaxEditor.setJustification (justification);
        outputMidiChannelEditor.setJustification (justification);
        outputNumEditor.setJustification (justification);
        // outputMsgTypeComboBox.setTextJustification (justification);
    }

    void setTitle (const juce::String& title)
    {
        titleLabel.setText (title, juce::dontSendNotification);
    }

    void setPath (const juce::String& path)
    {
        pathEditor.setText (path);
    }

    void setInputMin (auto min)
    {
        inputMinEditor.setText (juce::String (min));
    }

    void setInputMax (auto max)
    {
        inputMaxEditor.setText (juce::String (max));
    }

    void setOutputMidiChannel (int channel)
    {
        outputMidiChannelEditor.setText (juce::String (channel));
    }

    void setOutputNum (int num)
    {
        outputNumEditor.setText (juce::String (num));
    }

    void setOutputMsgType (int type)
    {
        outputMsgTypeComboBox.setSelectedId (type + 1);
    }

    void updateActivityForChan (auto& chan)
    {
        auto versionAtomic = chan.getLastValueVersionAtomic().load();
        auto& valueAtomic = chan.getLastValueAtomic();

        if (versionAtomic != lastValueVersion)
        {
            // New data
            activityIndicator.addValue (valueAtomic.load());
            lastValueVersion = versionAtomic;
        }
    }

    void addActivity (auto newValue)
    {
        activityIndicator.addValue (newValue);
    }

    void resized() override
    {
        const auto numElements = OSCBridgeChannelLabels::Labels::NumLabels;

        // Lay out with equal width from left to right
        auto area = getLocalBounds();
        auto width = area.getWidth() / numElements;

        auto titleWidth = width / 2;
        titleLabel.setBounds (area.removeFromLeft (titleWidth));
        pathEditor.setBounds (area.removeFromLeft (width));
        inputMinEditor.setBounds (area.removeFromLeft (width));
        inputMaxEditor.setBounds (area.removeFromLeft (width));
        activityIndicator.setBounds (area.removeFromLeft (width));
        outputMidiChannelEditor.setBounds (area.removeFromLeft (width));
        outputNumEditor.setBounds (area.removeFromLeft (width));
        outputMsgTypeComboBox.setBounds (area.removeFromLeft (width));
        muteButton.setBounds (area.removeFromLeft (width));
    }

    void setState (juce::ValueTree newState)
    {
        if (!newState.isValid())
        {
            return;
        }

        oscBridgeState = newState;

        // Update all the gui elements
        pathEditor.setText (newState.getProperty ("Path", ""));

        inputMinEditor.setText (juce::String (newState.getProperty ("InputMin", 0.0f)));
        inputMaxEditor.setText (juce::String (newState.getProperty ("InputMax", 1.0f)));
        outputMidiChannelEditor.setText (juce::String (newState.getProperty ("OutputMidiChannel", 1)));
        outputNumEditor.setText (juce::String (newState.getProperty ("OutputMidiNum", 1)));

        auto msgType = newState.getProperty ("MsgType", birdhouse::MsgType::MidiNote);
        outputMsgTypeComboBox.setSelectedItemIndex (static_cast<int> (msgType));
        muteButton.setToggleState (newState.getProperty ("Muted", false), juce::dontSendNotification);
    }

    auto& getActivityIndicator() { return activityIndicator; }

private:
    // GUI Components
    juce::Label titleLabel;
    juce::TextEditor pathEditor;
    juce::TextEditor inputMinEditor;
    juce::TextEditor inputMaxEditor;
    juce::TextEditor outputMidiChannelEditor;
    juce::TextEditor outputNumEditor;
    juce::ComboBox outputMsgTypeComboBox;
    juce::TextButton muteButton { "Mute" };

    ActivityIndicator<64> activityIndicator;

    juce::ValueTree oscBridgeState;

    int lastValueVersion { -1 };

    void updateChannelSettings (auto whatChanged)
    {
        if (oscBridgeState.isValid())
        {
            const auto pathID = juce::Identifier ("Path");
            const auto inputMinID = juce::Identifier ("InputMin");
            const auto inputMaxID = juce::Identifier ("InputMax");
            const auto outputMidiChannelID = juce::Identifier ("OutputMidiChannel");
            const auto outputMidiNumID = juce::Identifier ("OutputMidiNum");
            const auto msgTypeID = juce::Identifier ("MsgType");
            const auto mutedID = juce::Identifier ("Muted");

            if (whatChanged == pathID)
            {
                oscBridgeState.setProperty (pathID, pathEditor.getText(), nullptr);
            }
            else if (whatChanged == inputMinID)
            {
                oscBridgeState.setProperty (inputMinID, inputMinEditor.getText().getFloatValue(), nullptr);
            }
            else if (whatChanged == inputMaxID)
            {
                oscBridgeState.setProperty (inputMaxID, inputMaxEditor.getText().getFloatValue(), nullptr);
            }
            else if (whatChanged == outputMidiChannelID)
            {
                oscBridgeState.setProperty (outputMidiChannelID, outputMidiChannelEditor.getText().getIntValue(), nullptr);
            }
            else if (whatChanged == outputMidiNumID)
            {
                oscBridgeState.setProperty (outputMidiNumID, outputNumEditor.getText().getIntValue(), nullptr);
            }
            else if (whatChanged == msgTypeID)
            {
                oscBridgeState.setProperty (msgTypeID, outputMsgTypeComboBox.getSelectedId() - 1, nullptr);
            }
            else if (whatChanged == mutedID)
            {
                oscBridgeState.setProperty (mutedID, muteButton.getToggleState(), nullptr);
            }
        }
    }
};
