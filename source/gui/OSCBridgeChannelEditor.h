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

    void setInMin (auto min)
    {
        inputMinEditor.setText (juce::String (min));
    }

    void setInMax (auto max)
    {
        inputMaxEditor.setText (juce::String (max));
    }

    void setMidiChan (int channel)
    {
        outputMidiChannelEditor.setText (juce::String (channel));
    }

    void setMuted (bool muted)
    {
        muteButton.setToggleState (muted, juce::dontSendNotification);
    }

    void setMidiNum (int num)
    {
        outputNumEditor.setText (juce::String (num));
    }

    void setMsgType (int type)
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

    ActivityIndicator<64> activityIndicator;

    juce::ValueTree oscBridgeState;

    std::atomic<int> lastValueVersion { -1 };

    // Attachments
    std::vector<std::unique_ptr<birdhouse::ITextEditorAttachment>> textEditorAttachments {};

    // Setup button attachment
    juce::AudioProcessorValueTreeState::ButtonAttachment muteButtonAttachment {
        mApvts,
        "Muted" + juce::String (mChannelNum),
        muteButton
    };

    // MsgType
    juce::AudioProcessorValueTreeState::ComboBoxAttachment msgTypeAttachment {
        mApvts,
        "MsgType" + juce::String (mChannelNum),
        outputMsgTypeComboBox
    };

    void setupAttachments()
    {
        // Construct parameter ID strings based on channel number
        auto muteParamId = "Muted" + juce::String (mChannelNum);
        auto pathParamId = "Path" + juce::String (mChannelNum);
        auto inputMinParamId = "InMin" + juce::String (mChannelNum);
        auto inputMaxParamId = "InMax" + juce::String (mChannelNum);
        auto outputMidiChannelParamId = "MidiChan" + juce::String (mChannelNum);
        auto outputMidiNumParamId = "MidiNum" + juce::String (mChannelNum);
        auto msgTypeParamId = "MsgType" + juce::String (mChannelNum);

        // Set up attachments

        // Min
        textEditorAttachments.push_back (std::make_unique<birdhouse::TextEditorAttachment<float>> (
            mApvts, inputMinEditor, nullptr));
        textEditorAttachments.back()->attach (inputMinParamId);

        // Max
        textEditorAttachments.push_back (std::make_unique<birdhouse::TextEditorAttachment<float>> (
            mApvts, inputMaxEditor, nullptr));
        textEditorAttachments.back()->attach (inputMaxParamId);

        // outChan
        textEditorAttachments.push_back (std::make_unique<birdhouse::TextEditorAttachment<int>> (
            mApvts, outputMidiChannelEditor, nullptr));
        textEditorAttachments.back()->attach (outputMidiChannelParamId);

        // outNum
        textEditorAttachments.push_back (std::make_unique<birdhouse::TextEditorAttachment<int>> (
            mApvts, outputNumEditor, nullptr));
        textEditorAttachments.back()->attach (outputMidiNumParamId);

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

                // Set state
                auto identifier = juce::Identifier ("Path" + juce::String (mChannelNum));
                DBG ("GUI SETTING " << identifier << ": " << string);
                mApvts.state.setProperty (identifier, string, nullptr);
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
            const auto identifier = juce::Identifier ("InMin" + juce::String (mChannelNum));
            auto string = inputMinEditor.getText();

            if (string.isNotEmpty())
            {
                // Set state
                auto floatVal = inputMinEditor.getText().getFloatValue();
                DBG ("GUI SETTING " << identifier << ": " << floatVal);
                mApvts.state.setProperty (identifier, floatVal, nullptr);
            }
        };

        // Return -> focus lost
        inputMinEditor.onReturnKey = [&] {
            inputMinEditor.focusLost (FocusChangeType::focusChangedDirectly);
        };

        addAndMakeVisible (inputMaxEditor);
        inputMaxEditor.setInputRestrictions (5, "0123456789.-");

        // Filter text
        inputMaxEditor.onTextChange = [&] {
            const auto identifier = juce::Identifier ("InMax" + juce::String (mChannelNum));
            auto string = inputMaxEditor.getText();

            if (string.isNotEmpty())
            {
                // Set state
                auto floatVal = inputMaxEditor.getText().getFloatValue();
                DBG ("GUI SETTING " << identifier << ": " << floatVal);
                mApvts.state.setProperty (identifier, floatVal, nullptr);
            }
        };

        // Return -> focus lost
        inputMaxEditor.onReturnKey = [&] {
            inputMaxEditor.focusLost (FocusChangeType::focusChangedDirectly);
        };

        addAndMakeVisible (outputMidiChannelEditor);
        outputMidiChannelEditor.setInputRestrictions (2, "0123456789");

        // Return -> focus lost
        outputMidiChannelEditor.onReturnKey = [&] {
            outputMidiChannelEditor.focusLost (FocusChangeType::focusChangedDirectly);
        };

        addAndMakeVisible (outputNumEditor);
        outputNumEditor.setInputRestrictions (3, "0123456789");

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
