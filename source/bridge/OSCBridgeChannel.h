#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_osc/juce_osc.h>

class OSCBridgeChannel
{
public:
    enum MsgType {
        MidiNote,
        MidiCC,
        MidiBend
    };

    OSCBridgeChannel (const juce::String& path, float fromMin, float fromMax, int outputMidiChannel, int outputNum, MsgType outputType)
        : path (path), inputMin (fromMin), inputMax (fromMax), outputMidiChan (outputMidiChannel), outMidiNum (outputNum), msgType (outputType)
    {
    }

    // An alternative constructor that takes a ValueTree
    OSCBridgeChannel (juce::ValueTree channelState)
        : path (channelState.getProperty ("Path").toString()),
          inputMin (channelState.getProperty ("InputMin")),
          inputMax (channelState.getProperty ("InputMax")),
          outputMidiChan (channelState.getProperty ("OutputMidiChannel")),
          outMidiNum (channelState.getProperty ("OutputMidiNum")),
          msgType (static_cast<MsgType> (static_cast<int> (channelState.getProperty ("MsgType")))),
          muted (channelState.getProperty ("Muted", false))
    {
    }

    void setMuted (bool shouldBeMuted)
    {
        muted = shouldBeMuted;
    }

    // Setters
    void setPath (const juce::String& newPath)
    {
        path = newPath;
    }

    void setInputMin (auto newFromMin)
    {
        inputMin = newFromMin;
    }

    void setInputMax (auto newFromMax)
    {
        inputMax = newFromMax;
    }

    void setOutputMidiChannel (auto newOutputMidiChannel)
    {
        outputMidiChan = newOutputMidiChannel;
    }

    void setOutputMidiNum (auto newOutputNum)
    {
        juce::Logger::writeToLog ("Setting output num" + juce::String (newOutputNum));
        outMidiNum = newOutputNum;
    }

    void setOutputType (auto newOutputType)
    {
        msgType = newOutputType;
    }

    // Add MIDI message to the channel's list of buffers
    void addMidiMessageToBuffer (const juce::MidiMessage& message, int timeStamp = 0)
    {
        juce::Logger::writeToLog ("Adding MIDI message to buffer");
        mInternalBuffer.addEvent (message, timeStamp);

        const auto bufferSize = mInternalBuffer.getNumEvents();
        juce::Logger::writeToLog ("Buffer size: " + juce::String (bufferSize));
    }

    // Called at the start of each processBlock to move messages to the processBlock's midi buffer
    void appendMessagesTo (juce::MidiBuffer& processBlockBuffer)
    {
        if (mInternalBuffer.getNumEvents() > 0)
        {
            juce::Logger::writeToLog ("Transfering messages to processBlockBuffer");
            processBlockBuffer.addEvents (mInternalBuffer, 0, -1, 0);
            mInternalBuffer.clear(); // Flush the channel's MIDI messages after transfer
        }
    }

    auto matchesOSCAddress (const juce::String& address) const
    {
        return address == path;
    }

    void handleOSCMessage (const juce::OSCMessage& message)
    {
        if (!muted)
        {
            juce::Logger::writeToLog ("received message");

            if (message.size() == 1 && message[0].isFloat32())
            {
                juce::Logger::writeToLog ("received float");

                const auto rawValue = message[0].getFloat32();
                auto midiMessage = convertToMidiMessage (rawValue);
                juce::Logger::writeToLog ("MIDI message: " + midiMessage.getDescription());
                addMidiMessageToBuffer (midiMessage);
            }
        }
    }

private:
    juce::String path;
    float inputMin, inputMax;
    int outputMidiChan, outMidiNum;
    MsgType msgType;
    juce::MidiBuffer mInternalBuffer;

    bool muted;

    // Converts the raw value to a MIDI message
    juce::MidiMessage convertToMidiMessage (auto rawValue)
    {
        // Normalize the raw value to a 0-1 range
        float normalizedValue = juce::jmap (rawValue, inputMin, inputMax, 0.0f, 1.0f);
        juce::MidiMessage midiMessage;

        switch (msgType)
        {
            case MidiNote:
                midiMessage = (normalizedValue == 0.f) ? juce::MidiMessage::noteOff (outputMidiChan, outMidiNum, static_cast<uint8_t> (normalizedValue * 127))
                                                       : juce::MidiMessage::noteOn (outputMidiChan, outMidiNum, static_cast<uint8_t> (normalizedValue * 127));
                break;
            case MidiCC:
                midiMessage = juce::MidiMessage::controllerEvent (outputMidiChan, outMidiNum, static_cast<uint8_t> (normalizedValue * 127));
                break;
            case MidiBend:
                midiMessage = juce::MidiMessage::pitchWheel (outputMidiChan, static_cast<int> (normalizedValue * 16383) - 8192);
                break;
        }

        return midiMessage;
    }
};
