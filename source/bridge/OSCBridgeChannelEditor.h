#pragma once

#include "OSCBridgeChannel.h" // Include your OSCBridgeChannel header here
#include <juce_gui_basics/juce_gui_basics.h>

class OSCBridgeChannelEditor : public juce::Component
{
public:
    OSCBridgeChannelEditor()
    {
        // Setup components
        addAndMakeVisible (recvPortEditor);
        recvPortEditor.onTextChange = [this] { updateChannelSettings(); };

        addAndMakeVisible (pathEditor);
        pathEditor.onTextChange = [this] { updateChannelSettings(); };

        addAndMakeVisible (inputMinEditor);
        inputMinEditor.onTextChange = [this] { updateChannelSettings(); };

        addAndMakeVisible (inputMaxEditor);
        inputMaxEditor.onTextChange = [this] { updateChannelSettings(); };

        addAndMakeVisible (outputMidiChannelEditor);
        outputMidiChannelEditor.onTextChange = [this] { updateChannelSettings(); };

        addAndMakeVisible (outputNumEditor);
        outputNumEditor.onTextChange = [this] { updateChannelSettings(); };

        outputMsgTypeComboBox.addItem ("Note", OSCBridgeChannel::MidiNote + 1);
        outputMsgTypeComboBox.addItem ("CC", OSCBridgeChannel::MidiCC + 1);
        outputMsgTypeComboBox.addItem ("Bend", OSCBridgeChannel::MidiBend + 1);
        addAndMakeVisible (outputMsgTypeComboBox);
        outputMsgTypeComboBox.onChange = [this] { updateChannelSettings(); };

        outputMsgTypeComboBox.setSelectedId (OSCBridgeChannel::MidiNote + 1);

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
        recvPortEditor.setColour (colourID, colour);
        pathEditor.setColour (colourID, colour);
        inputMinEditor.setColour (colourID, colour);
        inputMaxEditor.setColour (colourID, colour);
        outputMidiChannelEditor.setColour (colourID, colour);
        outputNumEditor.setColour (colourID, colour);

        auto comboBoxID = juce::ComboBox::textColourId;
        outputMsgTypeComboBox.setColour (comboBoxID, colour);
    }

    void setFont (const juce::Font& font)
    {
        recvPortEditor.setFont (font);
        pathEditor.setFont (font);
        inputMinEditor.setFont (font);
        inputMaxEditor.setFont (font);
        outputMidiChannelEditor.setFont (font);
        outputNumEditor.setFont (font);
    }

    void setJustification (juce::Justification justification)
    {
        recvPortEditor.setJustification (justification);
        pathEditor.setJustification (justification);
        inputMinEditor.setJustification (justification);
        inputMaxEditor.setJustification (justification);
        outputMidiChannelEditor.setJustification (justification);
        outputNumEditor.setJustification (justification);
        // outputMsgTypeComboBox.setTextJustification (justification);
    }

    void setRecvPort (int port)
    {
        recvPortEditor.setText (juce::String (port));
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
        // Lay out with equal width from left to right
        auto area = getLocalBounds();
        auto width = area.getWidth() / 7;

        recvPortEditor.setBounds (area.removeFromLeft (width));
        pathEditor.setBounds (area.removeFromLeft (width));
        inputMinEditor.setBounds (area.removeFromLeft (width));
        inputMaxEditor.setBounds (area.removeFromLeft (width));
        outputMidiChannelEditor.setBounds (area.removeFromLeft (width));
        outputNumEditor.setBounds (area.removeFromLeft (width));
        outputMsgTypeComboBox.setBounds (area.removeFromLeft (width));
    }

private:
    // GUI Components
    juce::TextEditor recvPortEditor;
    juce::TextEditor pathEditor;
    juce::TextEditor inputMinEditor;
    juce::TextEditor inputMaxEditor;
    juce::TextEditor outputMidiChannelEditor;
    juce::TextEditor outputNumEditor;
    juce::ComboBox outputMsgTypeComboBox;

    // OSCBridgeChannel instance (assuming it's externally managed and passed to this editor)
    // OSCBridgeChannel* oscChannel;

    void updateChannelSettings()
    {
        // This method should update the OSCBridgeChannel instance based on the GUI components' values
        // Example: oscChannel->setPort(recvPortEditor.getText().getIntValue());
        // Note: You'll need to ensure oscChannel is properly initialized and accessible here.
    }
};
