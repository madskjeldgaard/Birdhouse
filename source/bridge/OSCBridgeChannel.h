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
        : mPath (path), mInputMin (fromMin), mInputMax (fromMax), mOutputMidiChan (outputMidiChannel), mOutMidiNum (outputNum), mMsgType (outputType)
    {
        mLastValueTime = juce::Time::currentTimeMillis();
    }

    // An alternative constructor that takes a ValueTree
    OSCBridgeChannel (juce::ValueTree channelState)
        : mPath (channelState.getProperty ("Path").toString()),
          mInputMin (channelState.getProperty ("InputMin")),
          mInputMax (channelState.getProperty ("InputMax")),
          mOutputMidiChan (channelState.getProperty ("OutputMidiChannel")),
          mOutMidiNum (channelState.getProperty ("OutputMidiNum")),
          mMsgType (static_cast<MsgType> (static_cast<int> (channelState.getProperty ("MsgType")))),
          muted (channelState.getProperty ("Muted", false))
    {
        mLastValueTime = juce::Time::currentTimeMillis();
    }

    void setMuted (bool shouldBeMuted)
    {
        muted = shouldBeMuted;
    }

    // Setters
    void setPath (const juce::String& newPath)
    {
        mPath = newPath;
    }

    void setInputMin (auto newFromMin)
    {
        mInputMin = newFromMin;
    }

    void setInputMax (auto newFromMax)
    {
        mInputMax = newFromMax;
    }

    void setOutputMidiChannel (auto newOutputMidiChannel)
    {
        mOutputMidiChan = newOutputMidiChannel;
    }

    void setOutputMidiNum (auto newOutputNum)
    {
        juce::Logger::writeToLog ("Setting output num" + juce::String (newOutputNum));
        mOutMidiNum = newOutputNum;
    }

    void setOutputType (auto newOutputType)
    {
        mMsgType = newOutputType;
    }

    auto getRawValue() const
    {
        return mRawValue;
    }

    /**
     * @brief Get the normalized value (0-1 range)
     *
     * @return auto
     */
    auto getNormalizedValue() const
    {
        return juce::jmap (mRawValue, mInputMin, mInputMax, 0.0f, 1.0f);
    }

    /**
     * @brief Get the time since the last value was received in milliseconds
     *
     * @return auto
     */
    auto timeSinceLastValue() const
    {
        auto currentTime = juce::Time::currentTimeMillis();

        auto timeSinceLastValue = currentTime - mLastValueTime;

        return timeSinceLastValue;
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
        return address == mPath;
    }

    void handleOSCMessage (const juce::OSCMessage& message)
    {
        if (!muted)
        {
            juce::Logger::writeToLog ("received message");
            mLastValueTime = juce::Time::currentTimeMillis();

            if (message.size() == 1 && (message[0].isFloat32() || message[0].isInt32()))
            {
                mRawValue = 0.f;

                if (message[0].isFloat32())
                {
                    mRawValue = message[0].getFloat32();
                }
                else if (message[0].isInt32())
                {
                    mRawValue = static_cast<float> (message[0].getInt32());
                }

                auto midiMessage = convertToMidiMessage (mRawValue);
                juce::Logger::writeToLog ("MIDI message: " + midiMessage.getDescription());
                addMidiMessageToBuffer (midiMessage);
            }
        }
    }

private:
    juce::int64 mLastValueTime { 0 }; // Tracks the last time a value > 0.0 was received

    juce::String mPath;
    float mInputMin { 0.f }, mInputMax { 1.0f }, mRawValue { 0.f };
    int mOutputMidiChan, mOutMidiNum;
    MsgType mMsgType;
    juce::MidiBuffer mInternalBuffer;

    bool muted;

    // Converts the raw value to a MIDI message
    juce::MidiMessage convertToMidiMessage (auto rawValue)
    {
        // Normalize the raw value to a 0-1 range
        float normalizedValue = juce::jmap (rawValue, mInputMin, mInputMax, 0.0f, 1.0f);
        juce::MidiMessage midiMessage;

        switch (mMsgType)
        {
            case MidiNote:
                midiMessage = (normalizedValue == 0.f) ? juce::MidiMessage::noteOff (mOutputMidiChan, mOutMidiNum, static_cast<uint8_t> (normalizedValue * 127))
                                                       : juce::MidiMessage::noteOn (mOutputMidiChan, mOutMidiNum, static_cast<uint8_t> (normalizedValue * 127));
                break;
            case MidiCC:
                midiMessage = juce::MidiMessage::controllerEvent (mOutputMidiChan, mOutMidiNum, static_cast<uint8_t> (normalizedValue * 127));
                break;
            case MidiBend:
                midiMessage = juce::MidiMessage::pitchWheel (mOutputMidiChan, static_cast<int> (normalizedValue * 16383) - 8192);
                break;
        }

        return midiMessage;
    }
};
