#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_osc/juce_osc.h>

namespace birdhouse
{
    enum MsgType {
        MidiCC,
        MidiNote,
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
                    midiMessage = (normalizedValue == 0.f) ? juce::MidiMessage::noteOff (outputMidiChannel, outputNum, static_cast<uint8_t> (normalizedValue * 127.f))
                                                           : juce::MidiMessage::noteOn (outputMidiChannel, outputNum, static_cast<uint8_t> (normalizedValue * 127.f));
                    break;
                case MsgType::MidiCC:
                    midiMessage = juce::MidiMessage::controllerEvent (outputMidiChannel, outputNum, static_cast<uint8_t> (normalizedValue * 127.f));
                    break;
                case MsgType::MidiBend:
                    midiMessage = juce::MidiMessage::pitchWheel (outputMidiChannel, static_cast<int> (normalizedValue * 16383) - 8192);
                    break;
                case MsgType::NumMsgTypes:
                    break;
                default:
                    break;
            }

            return midiMessage;
        }
    };

    /**
     * @class BridgeOSCMessageReceiver
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
            DBG ("Received OSC message: " + message.getAddressPattern().toString() + " with " + juce::String (message.size()) + " arguments");

            auto messageAccepted = message.size() == 1 && (message[0].isFloat32() || message[0].isInt32());
            auto rawValue = 0.f;

            DBG ("MSG accepted: " + juce::String (static_cast<int> (messageAccepted)));
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
     * @class BridgeMidiBufferManager
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
            DBG ("Adding MIDI message to buffer");
            DBG ("Message: " + message.getDescription());
            // FIXME: Without this lock, bad memory access segfaults will happen because the OSC thread is running realtime and writing to the midi buffer
            const juce::ScopedLock lock (mCriticalSection); // Lock while reading/modifying
            mInternalBuffer.addEvent (message, timeStamp);
        }

        // This is called at the start of each processBlock to move messages to the processBlock's midi buffer
        void appendMessagesTo (juce::MidiBuffer& processBlockBuffer, int sampleNum = 0)
        {
            // FIXME: Without this lock, bad memory access segfaults will happen because the OSC thread is running realtime and writing to the midi buffer
            const juce::ScopedLock lock (mCriticalSection); // Lock while reading/modifying
            processBlockBuffer.addEvents (mInternalBuffer, 0, -1, sampleNum);

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
            : mInputMin (fromMin), mInputMax (fromMax), mOutputMidiChan (outputMidiChannel), mOutMidiNum (outputNum), mMsgType (outputType)
        {
            setPath (path);
        }

        // Setters and getters
        void setPath (const juce::String& newPath)
        {
            DBG ("Changing path from " + mPath + " to " + newPath);
            mPath = newPath;
        }
        void setInputMin (float newFromMin)
        {
            DBG ("Changing input min from " + juce::String (mInputMin.load()) + " to " + juce::String (newFromMin) + " for path " + mPath);
            mInputMin = newFromMin;
        }

        void setInputMax (float newFromMax)
        {
            DBG ("Changing input max from " + juce::String (mInputMax.load()) + " to " + juce::String (newFromMax) + " for path " + mPath);
            mInputMax = newFromMax;
        }

        void setOutputMidiChannel (int newOutputMidiChannel)
        {
            DBG ("Changing output MIDI channel from " + juce::String (mOutputMidiChan) + " to " + juce::String (newOutputMidiChannel) + " for path " + mPath);
            mMidiChanged.store (true);
            mOutputMidiChan = newOutputMidiChannel;
        }

        void setOutputMidiNum (int newOutputNum)
        {
            DBG ("Changing output MIDI number from " + juce::String (mOutMidiNum) + " to " + juce::String (newOutputNum) + " for path " + mPath);
            mMidiChanged.store (true);
            mOutMidiNum = newOutputNum;
        }

        void setOutputType (MsgType newOutputType)
        {
            DBG ("Changing output type from " + juce::String (mMsgType) + " to " + juce::String (newOutputType) + " for path " + mPath);
            mMidiChanged.store (true);
            mMsgType = newOutputType;
        }

        void setMuted (bool shouldBeMuted)
        {
            DBG ("Changing muted from " + juce::String (static_cast<int> (mMuted)) + " to " + juce::String (static_cast<int> (shouldBeMuted)) + " for path " + mPath);
            mMuted = shouldBeMuted;
        }

        inline void setRawValue (float newValue)
        {
            DBG ("Changing raw value from " + juce::String (mRawValue.load()) + " to " + juce::String (newValue) + " for path " + mPath);
            mRawValue = newValue;
        }
        inline auto getRawValue() const { return mRawValue.load(); }

        inline auto normalizeValue (float rawValue) -> auto
        {
            return juce::jmap (rawValue, mInputMin.load(), mInputMax.load(), 0.0f, 1.0f);
        }

        inline auto getNormalizedValue() { return normalizeValue (mRawValue.load()); }

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

        inline auto midiChanged() const { return mMidiChanged.load(); }

        void resetMidiFlag()
        {
            mMidiChanged.store (false);
        }

    private:
        juce::String mPath { "" };
        std::atomic<bool> mMidiChanged { false };
        std::atomic<float> mInputMin { 0.f }, mInputMax { 1.0f };
        std::atomic<float> mRawValue { 0.f };
        int mOutputMidiChan { 1 }, mOutMidiNum { 48 };
        MsgType mMsgType { MsgType::MidiCC };
        bool mMuted { false };
    };

    class OSCBridgeChannel : public BridgeOSCMessageReceiver, public BridgeMidiBufferManager
    {
    public:
        OSCBridgeChannel (const juce::String& path, float fromMin, float fromMax, int outputMidiChannel, int outputNum, MsgType outputType)
            : mState (path, fromMin, fromMax, outputMidiChannel, outputNum, outputType)
        {
            DBG ("Contructing bridge channel");
            // Register callback that will be called when an OSC message is received
            this->addOSCCallback (
                [this] (float rawValue, bool messageAccepted, const juce::OSCMessage& oscMessage) {
                    juce::ignoreUnused (oscMessage);
                    mState.setRawValue (rawValue);

                    if (!mState.muted() && messageAccepted)
                    {
                        auto normalized = mState.getNormalizedValue();
                        auto midiMessage = MidiMessageConverter::floatToMidiMessage (normalized, mState.outChan(), mState.outNum(), mState.outType());
                        this->addMidiMessage (midiMessage);
                    }
                });

            DBG ("Set up bridge channel with path: " + mState.path() + " and output channel: " + juce::String (mState.outChan()) + " and output number: " + juce::String (mState.outNum()) + " and output type: " + juce::String (mState.outType()) + " and input min: " + juce::String (mState.inMin()) + " and input max: " + juce::String (mState.inMax()));
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
