#pragma once

#include "bridge/LambdaStateListener.h"
#include "bridge/OSCBridgeChannel.h"
#include "bridge/OSCBridgeManager.h"
#include "dsp/BirdHouseParams.h"
#include "dsp/SimpleNoiseGenerator.h"
#include <juce_audio_processors/juce_audio_processors.h>

static constexpr auto numBridgeChans = 8;

// #if (MSVC)
//     #include "ipps.h"
// #endif

class PluginProcessor : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener

{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    auto numOSCChannels() const { return mOscBridgeChannels.size(); }

    // MIDI
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    // Program
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    // OSC
    void tryConnect (auto port);
    auto& getChannel (std::size_t index) const { return mOscBridgeChannels.at (index); }

    inline auto isConnected()
    {
        return mConnected.load();
    }

    // State
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void updateListenerStates();
    void setStateChangeCallbacks();
    void updateChannelsFromParams();

    // Parameters
    juce::AudioProcessorValueTreeState parameters { *this, nullptr, "Parameters", birdhouse::BirdHouseParams<numBridgeChans>::createParameterLayout() };
    std::atomic<bool> mParametersNeedUpdating = true;
    void parameterChanged (const juce::String& parameterID, float newValue) override;

private:
    std::atomic<bool> mConnected = false;
    std::vector<std::shared_ptr<birdhouse::OSCBridgeChannel>> mOscBridgeChannels;
    std::shared_ptr<birdhouse::OSCBridgeManager> mOscBridgeManager;

    std::shared_ptr<LambdaStateListener> mGlobalStateListener;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
