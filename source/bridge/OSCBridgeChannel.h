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

    using OSCCallbackFunc = std::function<void (float, bool, const juce::OSCMessage&)>;

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

    auto normalizeValue (auto rawValue) -> auto
    {
        return juce::jmap (rawValue, mInputMin, mInputMax, 0.0f, 1.0f);
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
    auto getNormalizedValue()
    {
        return normalizeValue (mRawValue);
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
        mInternalBuffer.addEvent (message, timeStamp);

        const auto bufferSize = mInternalBuffer.getNumEvents();
    }

    // Called at the start of each processBlock to move messages to the processBlock's midi buffer
    void appendMessagesTo (juce::MidiBuffer& processBlockBuffer)
    {
        if (mInternalBuffer.getNumEvents() > 0)
        {
            processBlockBuffer.addEvents (mInternalBuffer, 0, -1, 0);
            mInternalBuffer.clear(); // Flush the channel's MIDI messages after transfer
        }
    }

    void addCallback (OSCCallbackFunc newCallback)
    {
        mCallbacks.push_back (std::move (newCallback));
    }

    auto matchesOSCAddress (const juce::String& address) const
    {
        return address == mPath;
    }

    void handleOSCMessage (const juce::OSCMessage& message)
    {
        auto messageAccepted = message.size() == 1 && (message[0].isFloat32() || message[0].isInt32());
        auto normalizedValue = 0.f;

        mLastValueTime = juce::Time::currentTimeMillis();

        // Retrieve the value from the message
        if (messageAccepted)
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

            normalizedValue = normalizeValue (mRawValue);

            // Update the atomic values, allowing safe access to latest data from the UI thread
            updateAtomic (normalizedValue);
        }

        // Call external callbacks
        for (auto& callback : mCallbacks)
        {
            callback (normalizedValue, messageAccepted, message);
        }

        // Send midi if not muted
        if (!muted)
        {
            auto midiMessage = convertToMidiMessage (normalizedValue);
            addMidiMessageToBuffer (midiMessage);
        }
    }

    inline auto& getLastValueAtomic()
    {
        return mLastValue;
    }

    inline auto& getLastValueVersionAtomic()
    {
        return mLastValueVersion;
    }

private:
    // Atomics for the channel's state
    std::atomic<float> mLastValue { 0.f };
    std::atomic<int> mLastValueVersion { 0 };

    inline void updateAtomic (float newValue)
    {
        mLastValue.store (newValue, std::memory_order_release);
        mLastValueVersion.store (mLastValueVersion.load() + 1, std::memory_order_release);
    }

    std::vector<OSCCallbackFunc> mCallbacks {};
    juce::int64 mLastValueTime { 0 }; // Tracks the last time a value > 0.0 was received
    juce::String mPath;
    float mInputMin { 0.f }, mInputMax { 1.0f }, mRawValue { 0.f };
    int mOutputMidiChan, mOutMidiNum;
    MsgType mMsgType;
    juce::MidiBuffer mInternalBuffer;

    bool muted;

    // Converts the raw value to a MIDI message
    juce::MidiMessage convertToMidiMessage (auto normalizedValue)
    {
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
