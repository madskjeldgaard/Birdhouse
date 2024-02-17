#pragma once

#include "OSCBridgeChannel.h"
#include <functional>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_osc/juce_osc.h>
namespace birdhouse
{
    /**
 * @class OSCBridgeManager
 * @brief The OSCBridgeManager class is responsible for managing the OSC bridge, it registers a callback for OSC and dispatches to all channels that are registered with it.
 *
 */
    class OSCBridgeManager : private juce::OSCReceiver, private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>
    {
    public:
        using GlobalOSCCallback = std::function<void (const juce::OSCMessage&)>;

        OSCBridgeManager (std::vector<std::shared_ptr<OSCBridgeChannel>> channels)
        {
            for (auto& channel : channels)
            {
                registerChannel (channel);

                // Add default callback
                channel->addOSCCallback ([&] (auto normalizedValue, auto valueAccepted, auto rawOSCMessage) {
                    juce::ignoreUnused (rawOSCMessage);
                    juce::Logger::writeToLog ("Normalized value: " + juce::String (normalizedValue) + " Value accepted: " + juce::String (static_cast<int> (valueAccepted)) + " Raw OSC message: " + rawOSCMessage.getAddressPattern().toString());
                });
            }

            addGlobalCallback ([&] (const juce::OSCMessage& message) {
                juce::Logger::writeToLog ("Global callback: " + message.getAddressPattern().toString());
            });

            mOscReceiver.addListener (this);
        }

        ~OSCBridgeManager() override
        {
            stopListening();
        }

        bool startListening (int port)
        {
            auto result = mOscReceiver.connect (port);
            DBG ("OSC Bridge Manager: startListening:" + juce::String (static_cast<int> (result)) + " on port:" + juce::String (port));
            return result;
        }

        void stopListening()
        {
            DBG ("OSC Bridge Manager: stopListening");
            mOscReceiver.disconnect();
        }

        void registerChannel (std::shared_ptr<OSCBridgeChannel> channel)
        {
            DBG ("Registering channel with path: " + channel->state().path());
            if (channel)
            {
                mChannels.emplace_back (channel);
            }
        }

        void addGlobalCallback (GlobalOSCCallback newCallback)
        {
            DBG ("OSC Bridge Manager: addGlobalCallback");
            mGlobalCallbacks.push_back (std::move (newCallback));
            DBG ("Num global callbacks:" + juce::String (mGlobalCallbacks.size()));
        }

        auto getChannels() const -> const std::vector<std::shared_ptr<OSCBridgeChannel>>&
        {
            return mChannels;
        }

        auto getChannel (auto num) -> std::shared_ptr<OSCBridgeChannel>&
        {
            return mChannels[num];
        }

    protected:
        void oscMessageReceived (const juce::OSCMessage& message) override
        {
            DBG ("Globally received OSC message:" + message.getAddressPattern().toString() + " with " + juce::String (message.size()) + " arguments");
            for (auto& callback : mGlobalCallbacks)
            {
                callback (message);
            }

            for (auto& channel : mChannels)
            {
                if (channel->matchesPath (message.getAddressPattern().toString()))
                {
                    channel->handleOSCMessage (message);
                }
            }
        }

    private:
        juce::OSCReceiver mOscReceiver;
        std::vector<std::shared_ptr<OSCBridgeChannel>> mChannels;
        std::vector<GlobalOSCCallback> mGlobalCallbacks {};
    };
}
