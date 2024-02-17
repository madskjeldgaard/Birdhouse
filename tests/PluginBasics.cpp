#include "helpers/test_helpers.h"
#include <PluginProcessor.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

TEST_CASE ("Plugin instance", "[instance]")
{
    PluginProcessor testPlugin;

    // This lets us use JUCE's MessageManager without leaking.
    // PluginProcessor might need this if you use the APVTS for example.
    // You'll also need it for tests that rely on juce::Graphics, juce::Timer, etc.
    auto gui = juce::ScopedJuceInitialiser_GUI {};

    SECTION ("name")
    {
        CHECK_THAT (testPlugin.getName().toStdString(),
            Catch::Matchers::Equals ("BirdHouse"));
    }

    auto numOSCChannels = testPlugin.numOSCChannels();
    SECTION ("number of OSC channels")
    {
        CHECK (numOSCChannels == 8);
    }

    SECTION ("OSC channel")
    {
        auto& channel = testPlugin.getChannel (0);

        // Set path
        channel.get()->state().setPath ("/1/value");
        CHECK (channel.get()->state().path() == "/1/value");

        // Set inMin
        channel.get()->state().setInputMin (0.0f);
        CHECK (channel.get()->state().inMin() == 0.0f);

        // Set inMax
        channel.get()->state().setInputMax (1.0f);
        CHECK (channel.get()->state().inMax() == 1.0f);

        // Set midiChan
        channel.get()->state().setOutputMidiChannel (1);
        CHECK (channel.get()->state().outChan() == 1);

        // Set midiNum
        channel.get()->state().setOutputMidiNum (1);
        CHECK (channel.get()->state().outNum() == 1);

        // Set msgType
        channel.get()->state().setOutputType (static_cast<birdhouse::MsgType> (1));
        CHECK (channel.get()->state().outType() == 1);

        // Set muted
        channel.get()->state().setMuted (false);
        CHECK_FALSE (channel.get()->state().muted());
    }
}

#ifdef PAMPLEJUCE_IPP
    #include <ipp.h>

TEST_CASE ("IPP version", "[ipp]")
{
    CHECK_THAT (ippsGetLibVersion()->Version, Catch::Matchers::Equals ("2021.10.1 (r0x8e799c51)"));
}
#endif
