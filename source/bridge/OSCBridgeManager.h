#pragma once

#include "OSCBridgeChannel.h"
#include <functional>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_osc/juce_osc.h>

/**
 * @class OSCBridgeManager
 * @brief The OSCBridgeManager class is responsible for managing the OSC bridge, it registers a callback for OSC and dispatches to all channels that are registered with it.
 *
 */
class OSCBridgeManager : private juce::OSCReceiver, private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>
{
public:
    OSCBridgeManager()
    {
        mOscReceiver.addListener (this);
    }

    ~OSCBridgeManager() override
    {
        stopListening();
    }

    bool startListening (int port)
    {
        return mOscReceiver.connect (port);
    }

    void stopListening()
    {
        mOscReceiver.disconnect();
    }

    void registerChannel (std::shared_ptr<OSCBridgeChannel> channel)
    {
        if (channel)
        {
            mChannels.emplace_back (channel);
        }
    }

    void addCallbackToChannel (std::size_t chanNum, auto newCallback)
    {
        if (chanNum < mChannels.size())
        {
            mChannels[chanNum]->addCallback (newCallback);
        }
    }

protected:
    void oscMessageReceived (const juce::OSCMessage& message) override
    {
        for (auto& channel : mChannels)
        {
            if (channel->matchesOSCAddress (message.getAddressPattern().toString()))
            {
                channel->handleOSCMessage (message);
            }
        }
    }

private:
    juce::OSCReceiver mOscReceiver;
    std::vector<std::shared_ptr<OSCBridgeChannel>> mChannels;
};
