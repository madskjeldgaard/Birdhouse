#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_osc/juce_osc.h>

class OSCBridgeChannel : private juce::OSCReceiver,
                         private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::RealtimeCallback>
{
public:
    enum OutputType {
        MidiNote,
        MidiCC,
        MidiBend
    };

    OSCBridgeChannel (int recvPort, const juce::String& path, float fromMin, float fromMax, int outputMidiChannel, int outputNum, OutputType outputType)
        : recvPort (recvPort), path (path), fromMin (fromMin), fromMax (fromMax), outputMidiChannel (outputMidiChannel), outputNum (outputNum), outputType (outputType)
    {
    }

    // In the destructor, we need to remove the listener and disconnect the OSCReceiver
    ~OSCBridgeChannel() override
    {
        stopListening();
    }

    void setPort (int port)
    {
        recvPort = port;
    }

    auto startListening()
    {
        if (!connect (recvPort))
        {
            juce::Logger::writeToLog ("Could not connect to port " + juce::String (recvPort));
            return false;
        }
        else
        {
            juce::Logger::writeToLog ("Connected to port " + juce::String (recvPort));
            addListener (this, path);
            return true;
        }
    }

    void stopListening()
    {
        removeListener (this);
        disconnect();
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
    void transferMessagesTo (juce::MidiBuffer& processBlockBuffer)
    {
        if (mInternalBuffer.getNumEvents() > 0)
        {
            juce::Logger::writeToLog ("Transfering messages to processBlockBuffer");
            processBlockBuffer.addEvents (mInternalBuffer, 0, -1, 0);
            mInternalBuffer.clear(); // Flush the channel's MIDI messages after transfer
        }
    }

protected:
    void oscMessageReceived (const juce::OSCMessage& message) override
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

private:
    int recvPort;
    juce::String path;
    float fromMin, fromMax;
    int outputMidiChannel, outputNum;
    OutputType outputType;
    juce::MidiBuffer mInternalBuffer;

    // Converts the raw value to a MIDI message
    juce::MidiMessage convertToMidiMessage (auto rawValue)
    {
        // Normalize the raw value to a 0-1 range
        float normalizedValue = juce::jmap (rawValue, fromMin, fromMax, 0.0f, 1.0f);
        juce::MidiMessage midiMessage;

        switch (outputType)
        {
            case MidiNote:
                midiMessage = (normalizedValue == 0.f) ? juce::MidiMessage::noteOff (outputMidiChannel, outputNum, static_cast<uint8_t> (normalizedValue * 127))
                                                       : juce::MidiMessage::noteOn (outputMidiChannel, outputNum, static_cast<uint8_t> (normalizedValue * 127));
                break;
            case MidiCC:
                midiMessage = juce::MidiMessage::controllerEvent (outputMidiChannel, outputNum, static_cast<uint8_t> (normalizedValue * 127));
                break;
            case MidiBend:
                midiMessage = juce::MidiMessage::pitchWheel (outputMidiChannel, static_cast<int> (normalizedValue * 16383) - 8192);
                break;
        }

        return midiMessage;
    }

    // Other private members...
};
