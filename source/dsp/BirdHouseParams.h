#pragma once

#include "../bridge/OSCBridgeChannel.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace birdhouse
{
    template <std::size_t NumBridgeChans = 8>
    class BirdHouseParams
    {
    public:
        static void addParameterListeners (juce::AudioProcessorValueTreeState& state, auto& processor)
        {
            // For each channel:
            // Add listener to each parameter
            for (auto chanNum = 1u; chanNum <= NumBridgeChans; chanNum++)
            {
                auto inMinParamID = juce::String ("InMin") + juce::String (chanNum);
                auto inMaxParamID = juce::String ("InMax") + juce::String (chanNum);
                auto midiChanParamID = juce::String ("MidiChan") + juce::String (chanNum);
                auto midiNumParamID = juce::String ("MidiNum") + juce::String (chanNum);
                auto msgTypeParamID = juce::String ("MsgType") + juce::String (chanNum);
                auto mutedParamID = juce::String ("Muted") + juce::String (chanNum);

                state.addParameterListener (inMinParamID, &processor);
                state.addParameterListener (inMaxParamID, &processor);
                state.addParameterListener (midiChanParamID, &processor);
                state.addParameterListener (midiNumParamID, &processor);
                state.addParameterListener (msgTypeParamID, &processor);
                state.addParameterListener (mutedParamID, &processor);
            }

            // Global parameters:
            // Add listener to port parameter
            state.addParameterListener ("Port", &processor);
        }

        static void removeParameterListeners (juce::AudioProcessorValueTreeState& state, auto& processor)
        {
            // For each channel:
            // Remove listener from each parameter
            for (auto chanNum = 1u; chanNum <= NumBridgeChans; chanNum++)
            {
                auto inMinParamID = juce::String ("InMin") + juce::String (chanNum);
                auto inMaxParamID = juce::String ("InMax") + juce::String (chanNum);
                auto midiChanParamID = juce::String ("MidiChan") + juce::String (chanNum);
                auto midiNumParamID = juce::String ("MidiNum") + juce::String (chanNum);
                auto msgTypeParamID = juce::String ("MsgType") + juce::String (chanNum);
                auto mutedParamID = juce::String ("Muted") + juce::String (chanNum);

                state.removeParameterListener (inMinParamID, &processor);
                state.removeParameterListener (inMaxParamID, &processor);
                state.removeParameterListener (midiChanParamID, &processor);
                state.removeParameterListener (midiNumParamID, &processor);
                state.removeParameterListener (msgTypeParamID, &processor);
                state.removeParameterListener (mutedParamID, &processor);
            }

            // Global parameters:
            // Remove listener from port parameter
            state.removeParameterListener ("Port", &processor);
        }

        // This will set up all the non-Audio Parameter parameters (such as port, inMin, inMax) in the state of the AudioProcessorValueTreeState;
        static void setupNonAudioParameters (juce::AudioProcessorValueTreeState& state)
        {
            for (auto chanNum = 1u; chanNum <= NumBridgeChans; chanNum++)
            {
                const auto inMinParamID = juce::Identifier (juce::String ("InMin") + juce::String (chanNum));
                const auto inMaxParamID = juce::Identifier (juce::String ("InMax") + juce::String (chanNum));
                const auto pathParamID = juce::Identifier (juce::String ("Path") + juce::String (chanNum));

                state.state.setProperty (inMinParamID, 0.0f, nullptr);
                state.state.setProperty (inMaxParamID, 1.0f, nullptr);
                state.state.setProperty (pathParamID, "/" + juce::String (chanNum) + "/value", nullptr);
            }
        }

        static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
        {
            // Add port parameter
            juce::AudioProcessorValueTreeState::ParameterLayout portLayout;

            auto maxPort = 65535;
            portLayout.add (std::make_unique<juce::AudioParameterInt> ("Port", "Port", 0, maxPort, 8000));

            for (auto chanNum = 1u; chanNum <= NumBridgeChans; chanNum++)
            {
                // Add InMin parameter for this channel
                // auto inMinParamID = juce::String ("InMin") + juce::String (chanNum);
                // auto inMinParameterName = juce::String ("InMin") + juce::String (chanNum);
                // auto inMinMinValue = 0.f;
                // auto inMinMaxValue = 20000.f;
                // auto inMinDefaultValue = 0.f;
                // auto inMinIntervalValue = 0.e1f;
                // auto normaliseableRange = juce::NormalisableRange<float> (inMinMinValue, inMinMaxValue);
                // portLayout.add (std::make_unique<juce::AudioParameterFloat> (inMinParamID, inMinParameterName, normaliseableRange, inMinDefaultValue));

                // Add InMax parameter
                // auto inMaxParamID = juce::String ("InMax") + juce::String (chanNum);
                // auto inMaxParameterName = juce::String ("InMax") + juce::String (chanNum);
                // auto inMaxMinValue = 0.f;
                // auto inMaxMaxValue = 20000.f;
                // auto inMaxDefaultValue = 1.f;
                // portLayout.add (std::make_unique<juce::AudioParameterFloat> (inMaxParamID, inMaxParameterName, inMaxMinValue, inMaxMaxValue, inMaxDefaultValue));

                // Add MIDI Chan parameter
                auto midiChanParamID = juce::String ("MidiChan") + juce::String (chanNum);
                auto midiChanParameterName = juce::String ("MidiChan") + juce::String (chanNum);
                auto midiChanMinValue = 1;
                auto midiChanMaxValue = 16;
                auto midiChanDefaultValue = 1;
                portLayout.add (std::make_unique<juce::AudioParameterInt> (midiChanParamID, midiChanParameterName, midiChanMinValue, midiChanMaxValue, midiChanDefaultValue));

                // Add MIDI Num parameter
                auto midiNumParamID = juce::String ("MidiNum") + juce::String (chanNum);
                auto midiNumParameterName = juce::String ("MidiNum") + juce::String (chanNum);
                auto midiNumMinValue = 0;
                auto midiNumMaxValue = 127;
                auto midiNumDefaultValue = 48 + chanNum - 1;
                portLayout.add (std::make_unique<juce::AudioParameterInt> (midiNumParamID, midiNumParameterName, midiNumMinValue, midiNumMaxValue, midiNumDefaultValue));

                // Add MsgType parameter
                auto msgTypeParamID = juce::String ("MsgType") + juce::String (chanNum);
                auto msgTypeParameterName = juce::String ("MsgType") + juce::String (chanNum);
                auto msgTypeMinValue = 0;
                auto msgTypeMaxValue = birdhouse::MsgType::NumMsgTypes - 1;
                ;
                auto msgTypeDefaultValue = 0;
                portLayout.add (std::make_unique<juce::AudioParameterInt> (msgTypeParamID, msgTypeParameterName, msgTypeMinValue, msgTypeMaxValue, msgTypeDefaultValue));

                // Add Muted parameter
                auto mutedParamID = juce::String ("Muted") + juce::String (chanNum);
                auto mutedParameterName = juce::String ("Muted") + juce::String (chanNum);
                auto mutedDefaultValue = false;
                portLayout.add (std::make_unique<juce::AudioParameterBool> (mutedParamID, mutedParameterName, mutedDefaultValue));
            }

            return portLayout;
        }
    };
}
