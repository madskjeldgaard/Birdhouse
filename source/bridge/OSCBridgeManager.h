#pragma once

#include "OSCBridgeChannel.h"
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
        oscReceiver.addListener (this);
    }

    // ~OSCBridgeManager() override
    // {
    //     oscReceiver.removeListener (this);
    // }

    bool startListening (int port)
    {
        return oscReceiver.connect (port);
    }

    void stopListening()
    {
        oscReceiver.disconnect();
    }

    void registerChannel (std::shared_ptr<OSCBridgeChannel> channel)
    {
        if (channel)
        {
            channels.emplace_back (channel);
        }
    }

protected:
    void oscMessageReceived (const juce::OSCMessage& message) override
    {
        for (auto& channel : channels)
        {
            if (channel->matchesOSCAddress (message.getAddressPattern().toString()))
            {
                channel->handleOSCMessage (message);
            }
        }
    }

private:
    juce::OSCReceiver oscReceiver;
    std::vector<std::shared_ptr<OSCBridgeChannel>> channels;
};
