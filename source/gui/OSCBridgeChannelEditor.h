#pragma once

#include "../bridge/OSCBridgeChannel.h"
#include "ActivityIndicator.h"
#include "OSCBridgeChannelLabels.h"
#include "TextEditorAttachment.h"
#include <juce_gui_basics/juce_gui_basics.h>

class OSCBridgeChannelEditor : public juce::Component
{
public:
    OSCBridgeChannelEditor (std::size_t channelNum, auto& apvts) : mChannelNum (channelNum), mApvts (apvts)
    {
        setupUI();
        setupAttachments();
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
        auto version = chan.getLastValueVersionAtomic().load();
        auto& valueAtomic = chan.getLastValueAtomic();

        if (version != lastValueVersion)
        {
            // New data
            activityIndicator.addValue (valueAtomic.load());
            lastValueVersion.store (version, std::memory_order_relaxed);
            juce::Logger::writeToLog ("new act");
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

    auto& getActivityIndicator() { return activityIndicator; }

    enum class AttachmentType {
        Path,
        InputMin,
        InputMax,
        OutputMidiChannel,
        OutputMidiNum,
        MsgType,
        Muted
    };

private:
    std::size_t mChannelNum { 0 };
    juce::AudioProcessorValueTreeState& mApvts;

    // GUI Components
    juce::Label titleLabel;
    juce::TextEditor pathEditor;
    juce::TextEditor inputMinEditor;
    juce::TextEditor inputMaxEditor;
    juce::TextEditor outputMidiChannelEditor;
    juce::TextEditor outputNumEditor;
    juce::ComboBox outputMsgTypeComboBox;
    juce::TextButton muteButton { "Mute" };

    // Attachments for the text editors
    birdhouse::TextEditorAttachment<juce::AudioParameterFloat> inputMinAttachment;
    birdhouse::TextEditorAttachment<juce::AudioParameterFloat> inputMaxAttachment;
    birdhouse::TextEditorAttachment<juce::AudioParameterInt> outputMidiChannelAttachment;
    birdhouse::TextEditorAttachment<juce::AudioParameterInt> outputNumAttachment;
    juce::AudioProcessorValueTreeState::ButtonAttachment muteButtonAttachment;

    ActivityIndicator<64> activityIndicator;

    juce::ValueTree oscBridgeState;

    std::atomic<int> lastValueVersion { -1 };

    void setupAttachments()
    {
        // Construct parameter ID strings based on channel number
        auto muteParamId = juce::Identifier ("mute" + juce::String (mChannelNum));
        auto pathParamId = juce::Identifier ("path" + juce::String (mChannelNum));
        auto inputMinParamId = juce::Identifier ("inputMin" + juce::String (mChannelNum));
        auto inputMaxParamId = juce::Identifier ("inputMax" + juce::String (mChannelNum));
        auto outputMidiChannelParamId = juce::Identifier ("outputMidiChannel" + juce::String (mChannelNum));
        auto outputMidiNumParamId = juce::Identifier ("outputMidiNum" + juce::String (mChannelNum));
        auto msgTypeParamId = juce::Identifier ("msgType" + juce::String (mChannelNum));

        // Set up attachments

        // Min
        auto inMin = std::make_unique<birdhouse::TextEditorAttachment<juce::AudioParameterFloat>> (
            mApvts.getParameter (inputMinParamId), inputMinEditor, nullptr);

        // Max
        auto inMax = std::make_unique<birdhouse::TextEditorAttachment<juce::AudioParameterFloat>> (
            mApvts.getParameter (inputMaxParamId), inputMaxEditor, nullptr);

        // Midi channel
        auto midiChan = std::make_unique<birdhouse::TextEditorAttachment<juce::AudioParameterInt>> (
            mApvts.getParameter (outputMidiChannelParamId), outputMidiChannelEditor, nullptr);

        // Midi num
        auto midiNum = std::make_unique<birdhouse::TextEditorAttachment<juce::AudioParameterInt>> (
            mApvts.getParameter (outputMidiNumParamId), outputNumEditor, nullptr);

        // TODO:
        // Msg type
        // auto msgType = std::make_unique<birdhouse::TextEditorAttachment<juce::AudioParameterInt>> (
        //     apvts.getParameter (msgTypeParamId), outputMsgTypeComboBox, nullptr);

        // TODO:
        // // Muted
        // auto muted = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        //     apvts.getParameter (muteParamId), muteButton);
    }

    void setupUI()
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

        addAndMakeVisible (inputMinEditor);
        inputMinEditor.setInputRestrictions (5, "0123456789.-");

        // text change-> filter text
        inputMinEditor.onTextChange = [&] {
        };

        // Return -> focus lost
        inputMinEditor.onReturnKey = [&] {
            inputMinEditor.focusLost (FocusChangeType::focusChangedDirectly);
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

        outputMsgTypeComboBox.setColour (juce::ComboBox::arrowColourId, BirdHouse::Colours::fg);

        outputMsgTypeComboBox.addItem ("Note", birdhouse::MsgType::MidiNote + 1);
        outputMsgTypeComboBox.addItem ("CC", birdhouse::MsgType::MidiCC + 1);
        // outputMsgTypeComboBox.addItem ("Bend", OSCBridgeChannel::MidiBend + 1);
        addAndMakeVisible (outputMsgTypeComboBox);
        // TODO:
        outputMsgTypeComboBox.onChange = [&] {
            const auto whatChanged = juce::Identifier ("MsgType");
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

        // Set text justification
        auto justification = juce::Justification::left;
        setJustification (justification);

        // ActivityIndicator
        addAndMakeVisible (activityIndicator);

        addAndMakeVisible (titleLabel);
        titleLabel.setColour (juce::Label::ColourIds::textColourId, BirdHouse::Colours::blue);
        titleLabel.setJustificationType (juce::Justification::centred);
    }
};
