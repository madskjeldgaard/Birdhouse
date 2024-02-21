#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_osc/juce_osc.h>

namespace birdhouse
{
    enum MsgType {
        MidiNote,
        MidiCC,
        MidiBend,
        NumMsgTypes
    };

    /**
     * @class MidiMessageConverter
     * @brief Converts a normalized value to a MIDI message
     *
     */
    class MidiMessageConverter
    {
    public:
        static juce::MidiMessage floatToMidiMessage (float normalizedValue, int outputMidiChannel, int outputNum, MsgType msgType)
        {
            juce::MidiMessage midiMessage;

            switch (msgType)
            {
                case MsgType::MidiNote:
                    midiMessage = (normalizedValue == 0.f) ? juce::MidiMessage::noteOff (outputMidiChannel, outputNum, static_cast<uint8_t> (normalizedValue * 127))
                                                           : juce::MidiMessage::noteOn (outputMidiChannel, outputNum, static_cast<uint8_t> (normalizedValue * 127));
                    break;
                case MsgType::MidiCC:
                    midiMessage = juce::MidiMessage::controllerEvent (outputMidiChannel, outputNum, static_cast<uint8_t> (normalizedValue * 127));
                    break;
                case MsgType::MidiBend:
                    midiMessage = juce::MidiMessage::pitchWheel (outputMidiChannel, static_cast<int> (normalizedValue * 16383) - 8192);
                    break;
                default:
                    break;
            }

            return midiMessage;
        }
    };

    /**
     * @class OSCMessageReceiver
     * @brief Responsible for keeping track of OSC callbacks and dispatching messages to them
     *
     */
    class BridgeOSCMessageReceiver
    {
    public:
        using OSCCallbackFunc = std::function<void (float, bool, const juce::OSCMessage&)>;
        void addOSCCallback (OSCCallbackFunc newCallback)
        {
            mCallbacks.push_back (std::move (newCallback));
        }
        void handleOSCMessage (const juce::OSCMessage& message)
        {
            auto messageAccepted = message.size() == 1 && (message[0].isFloat32() || message[0].isInt32());
            auto rawValue = 0.f;

            // Retrieve the value from the message
            if (messageAccepted)
            {
                rawValue = 0.f;

                if (message[0].isFloat32())
                {
                    rawValue = message[0].getFloat32();
                }
                else if (message[0].isInt32())
                {
                    rawValue = static_cast<float> (message[0].getInt32());
                }
            }

            // Call external callbacks
            for (auto& callback : mCallbacks)
            {
                callback (rawValue, messageAccepted, message);
            }
        }

    private:
        std::vector<OSCCallbackFunc> mCallbacks {};
    };

    /**
     * @class MidiBufferManager
     * @brief Manages a buffer of MIDI messages
     *
     */
    class BridgeMidiBufferManager
    {
    public:
        BridgeMidiBufferManager()
        {
            mInternalBuffer.clear();
        }

        // This is called by the OSC part of the plugin
        void addMidiMessage (const juce::MidiMessage& message, int timeStamp = 0)
        {
            const juce::ScopedLock lock (mCriticalSection); // Lock while reading/modifying
            mInternalBuffer.addEvent (message, timeStamp);
        }

        // This is called at the start of each processBlock to move messages to the processBlock's midi buffer
        void appendMessagesTo (juce::MidiBuffer& processBlockBuffer)
        {
            // FIXME: This causes bad memory access
            const juce::ScopedLock lock (mCriticalSection); // Lock while reading/modifying
            processBlockBuffer.addEvents (mInternalBuffer, 0, -1, 0);

            mInternalBuffer.clear();
        }

    private:
        // FIXME: Temporary
        juce::CriticalSection mCriticalSection; // Replaces std::mutex

        juce::MidiBuffer mInternalBuffer {};
    };

    /**
 * @class OSCBridgeChannelState
 * @brief Manages state of each channel
 *
 */
    class OSCBridgeChannelState
    {
        // Constructor
    public:
        OSCBridgeChannelState (const juce::String& path, float fromMin, float fromMax, int outputMidiChannel, int outputNum, MsgType outputType)
            : mPath (path), mInputMin (fromMin), mInputMax (fromMax), mOutputMidiChan (outputMidiChannel), mOutMidiNum (outputNum), mMsgType (outputType)
        {
        }

        // Setters and getters
        void setPath (const juce::String& newPath) { mPath = newPath; }
        void setInputMin (auto newFromMin) { mInputMin = newFromMin; }

        void setInputMax (auto newFromMax) { mInputMax = newFromMax; }

        void setOutputMidiChannel (auto newOutputMidiChannel) { mOutputMidiChan = newOutputMidiChannel; }

        void setOutputMidiNum (auto newOutputNum) { mOutMidiNum = newOutputNum; }

        void setOutputType (auto newOutputType) { mMsgType = newOutputType; }

        void setMuted (bool shouldBeMuted) { mMuted = shouldBeMuted; }

        void setRawValue (auto newValue) { mRawValue = newValue; }
        auto getRawValue() const { return mRawValue.load(); }

        inline auto& getLastValueAtomic() { return mRawValue; }
        inline auto& getLastValueVersionAtomic() { return mLastValueVersion; }

        auto normalizeValue (auto rawValue) -> auto
        {
            return juce::jmap (rawValue, mInputMin.load(), mInputMax.load(), 0.0f, 1.0f);
        }

        auto getNormalizedValue() { return normalizeValue (mRawValue.load()); }

        auto muted() const
        {
            return mMuted;
        }

        auto inMin() const
        {
            return mInputMin.load();
        }

        auto inMax() const
        {
            return mInputMax.load();
        }

        auto outChan() const
        {
            return mOutputMidiChan;
        }

        auto outNum() const
        {
            return mOutMidiNum;
        }

        auto outType() const
        {
            return mMsgType;
        }

        auto path() const
        {
            return mPath;
        }

    private:
        juce::String mPath;
        std::atomic<float> mInputMin { 0.f }, mInputMax { 1.0f };
        std::atomic<float> mRawValue { 0.f };
        std::atomic<int> mLastValueVersion { 0 };
        int mOutputMidiChan, mOutMidiNum;
        MsgType mMsgType;
        bool mMuted;
    };

    class OSCBridgeChannel : public BridgeOSCMessageReceiver, public BridgeMidiBufferManager
    {
    public:
        OSCBridgeChannel (const juce::String& path, float fromMin, float fromMax, int outputMidiChannel, int outputNum, MsgType outputType)
            : mState (path, fromMin, fromMax, outputMidiChannel, outputNum, outputType)
        {
            // Register callback that will be called when an OSC message is received
            this->addOSCCallback (
                [this] (float rawValue, bool messageAccepted, const juce::OSCMessage& message) {
                    juce::ignoreUnused (message);
                    mState.setRawValue (rawValue);

                    if (!mState.muted() && messageAccepted)
                    {
                        auto normalized = mState.getNormalizedValue();
                        auto midiMessage = MidiMessageConverter::floatToMidiMessage (normalized, mState.outChan(), mState.outNum(), mState.outType());
                        this->addMidiMessage (midiMessage);
                    }
                });
        }

        auto& state() { return mState; }

        auto matchesPath (const juce::String& address) const
        {
            return address == mState.path();
        }

    private:
        OSCBridgeChannelState mState;
    };

}
