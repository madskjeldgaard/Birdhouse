#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class OSCBridgeChannelLabels : public juce::Component
{
public:
    enum Labels {
        TitleLabel,
        Path,
        InputMin,
        InputMax,
        Activity,
        MidiChannel,
        MidiNum,
        OutputMsgType,
        Muted,
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

        // Title / Chan number
        labels[TitleLabel]->setText ("", juce::dontSendNotification);

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

        // muted
        labels[Muted]->setText ("", juce::dontSendNotification);
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
        auto titleWidth = width / 2;
        auto muteWidth = width / 2;

        // Readjust total width to account for the title and mute button
        width = (area.getWidth() - titleWidth - muteWidth) / (NumLabels - 2);

        auto labelNum = 0;
        for (auto& label : labels)
        {
            if (labelNum == TitleLabel)
            {
                label->setBounds (area.removeFromLeft (titleWidth));
            }
            else if (labelNum == Muted)
            {
                label->setBounds (area.removeFromLeft (muteWidth));
            }
            else
            {
                label->setBounds (area.removeFromLeft (width));
            }

            labelNum++;
        }
    }

private:
    std::vector<std::unique_ptr<juce::Label>> labels {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCBridgeChannelLabels)
};
