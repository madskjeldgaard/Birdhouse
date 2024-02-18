#pragma once

#include "../bridge/OSCBridgeChannel.h" // Include your OSCBridgeChannel header here
#include "ActivityIndicator.h"
#include "OSCBridgeChannelLabels.h" // Include your OSCBridgeChannel header here
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
        pathEditor.onTextChange = [this] {
            const auto whatChanged = juce::Identifier ("Path");
            updateChannelSettings (whatChanged);
        };

        addAndMakeVisible (inputMinEditor);
        inputMinEditor.onTextChange = [this] {
            const auto whatChanged = juce::Identifier ("InputMin");
            updateChannelSettings (whatChanged);
        };

        addAndMakeVisible (inputMaxEditor);
        inputMaxEditor.onTextChange = [this] {
            const auto whatChanged = juce::Identifier ("InputMax");
            updateChannelSettings (whatChanged);
        };

        addAndMakeVisible (outputMidiChannelEditor);
        outputMidiChannelEditor.onTextChange = [this] {
            const auto whatChanged = juce::Identifier ("OutputMidiChannel");
            updateChannelSettings (whatChanged);
        };

        addAndMakeVisible (outputNumEditor);
        outputNumEditor.onTextChange = [this] {
            const auto whatChanged = juce::Identifier ("OutputMidiNum");
            updateChannelSettings (whatChanged);
        };

        outputMsgTypeComboBox.addItem ("Note", OSCBridgeChannel::MidiNote + 1);
        outputMsgTypeComboBox.addItem ("CC", OSCBridgeChannel::MidiCC + 1);
        // outputMsgTypeComboBox.addItem ("Bend", OSCBridgeChannel::MidiBend + 1);
        addAndMakeVisible (outputMsgTypeComboBox);
        outputMsgTypeComboBox.onChange = [this] {
            const auto whatChanged = juce::Identifier ("MsgType");
            updateChannelSettings (whatChanged);
        };

        outputMsgTypeComboBox.setSelectedId (OSCBridgeChannel::MidiNote + 1);

        addAndMakeVisible (muteButton);
        muteButton.setToggleable (true);
        muteButton.setClickingTogglesState (true);
        muteButton.setButtonText ("mute");
        muteButton.setColour (juce::TextButton::buttonColourId, BirdHouse::Colours::bg);
        muteButton.setColour (juce::TextButton::buttonOnColourId, BirdHouse::Colours::red);
        // Set up toggle button callback
        muteButton.onClick = [this] {
            const auto whatChanged = juce::Identifier ("Muted");
            updateChannelSettings (whatChanged);
        };

        // Set text justification
        auto justification = juce::Justification::left;
        setJustification (justification);

        // outputMsgTypeComboBox.setTextJustification (justification);

        // Initialize your OSCBridgeChannel instance here if needed

        // TODO:
        // Add callbacks
    }

    void setColour (juce::Colour colour)
    {
        const auto colourID = juce::Label::textColourId;
        inputMinEditor.setColour (colourID, colour);
        inputMaxEditor.setColour (colourID, colour);
        outputMidiChannelEditor.setColour (colourID, colour);
        outputNumEditor.setColour (colourID, colour);
        // muteButton.setColour (juce::TextButton::buttonColourId, colour);

        auto comboBoxID = juce::ComboBox::textColourId;
        outputMsgTypeComboBox.setColour (comboBoxID, colour);
    }

    // Amount = -1.0 to 1.0
    // Below 0.0, it paints the path in textColourId
    // At 0.0, it paints the path in red
    // Above 0.0, it paints the path in green
    // void setActivityAmount (float amount)
    // {
    //     // activityIndicator.setAmount (amount);
    //     // activityIndicator.repaint();
    //     //
    //     if (amount < 0.0f)
    //     {
    //         pathEditor.setColour (juce::TextEditor::textColourId, BirdHouse::Colours::fg);
    //     }
    //     else if (amount == 0.0f)
    //     {
    //         pathEditor.setColour (juce::TextEditor::textColourId, BirdHouse::Colours::red);
    //     }
    //     else
    //     {
    //         pathEditor.setColour (juce::TextEditor::textColourId, BirdHouse::Colours::green);
    //     }

    //     pathEditor.repaint();
    // }

    void setFont (const juce::Font& font)
    {
        pathEditor.setFont (font);
        inputMinEditor.setFont (font);
        inputMaxEditor.setFont (font);
        outputMidiChannelEditor.setFont (font);
        outputNumEditor.setFont (font);
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

    void resized() override
    {
        const auto numElements = OSCBridgeChannelLabels::Labels::NumLabels;

        // Lay out with equal width from left to right
        auto area = getLocalBounds();
        auto width = area.getWidth() / numElements;

        pathEditor.setBounds (area.removeFromLeft (width));
        inputMinEditor.setBounds (area.removeFromLeft (width));
        inputMaxEditor.setBounds (area.removeFromLeft (width));
        outputMidiChannelEditor.setBounds (area.removeFromLeft (width));
        outputNumEditor.setBounds (area.removeFromLeft (width));
        outputMsgTypeComboBox.setBounds (area.removeFromLeft (width));
        muteButton.setBounds (area.removeFromLeft (width));
        // activityIndicator.setBounds (area.removeFromLeft (width));
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
        // TODO:
        // outputMsgTypeComboBox.setSelectedId (newState.getProperty ("MsgType", OSCBridgeChannel::MidiNote).getObject() + 1);
        muteButton.setToggleState (newState.getProperty ("Muted", false), juce::dontSendNotification);
    }

private:
    // GUI Components
    // ActivityIndicator activityIndicator;
    juce::TextEditor pathEditor;
    juce::TextEditor inputMinEditor;
    juce::TextEditor inputMaxEditor;
    juce::TextEditor outputMidiChannelEditor;
    juce::TextEditor outputNumEditor;
    juce::ComboBox outputMsgTypeComboBox;
    juce::TextButton muteButton { "Mute" };

    juce::ValueTree oscBridgeState;

    // OSCBridgeChannel instance (assuming it's externally managed and passed to this editor)
    // OSCBridgeChannel* oscChannel;

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
