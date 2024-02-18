#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class OSCBridgeChannelLabels : public juce::Component
{
public:
    enum Labels {
        Path,
        InputMin,
        InputMax,
        MidiChannel,
        MidiNum,
        OutputMsgType,
        Activity,
        NumLabels
    };

    // Constructor
    OSCBridgeChannelLabels()
    {
        // Init labels
        for (auto i = 0; i < NumLabels; ++i)
        {
            labels.push_back (std::make_unique<juce::Label>());
        }

        for (auto& label : labels)
        {
            addAndMakeVisible (label.get());
        }

        // Activity
        labels[Activity]->setText ("", juce::dontSendNotification);

        // Path
        labels[Path]->setText ("Path", juce::dontSendNotification);

        // InputMin
        labels[InputMin]->setText ("InMin", juce::dontSendNotification);

        // InputMax
        labels[InputMax]->setText ("InMax", juce::dontSendNotification);

        // OutputMidiChannel
        labels[MidiChannel]->setText ("MIDIChan", juce::dontSendNotification);

        // OutputNum
        labels[MidiNum]->setText ("MIDINum", juce::dontSendNotification);

        // OutputMsgType
        labels[OutputMsgType]->setText ("MsgType", juce::dontSendNotification);
    }

    void setFont (const juce::Font& font)
    {
        for (auto& label : labels)
        {
            label->setFont (font);
        }
    }

    void setColour (juce::Colour colour)
    {
        for (auto& label : labels)
        {
            label->setColour (juce::Label::textColourId, colour);
        }
    }

    void resized() override
    {
        // Lay out with equal width from left to right
        auto area = getLocalBounds();
        auto width = area.getWidth() / NumLabels;

        for (auto& label : labels)
        {
            label->setBounds (area.removeFromLeft (width));
        }
    }

private:
    std::vector<std::unique_ptr<juce::Label>> labels {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCBridgeChannelLabels)
};
