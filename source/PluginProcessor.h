#pragma once

#include "bridge/LambdaStateListener.h"
#include "bridge/OSCBridgeChannel.h"
#include "bridge/OSCBridgeManager.h"
#include "dsp/SimpleNoiseGenerator.h"
#include <juce_audio_processors/juce_audio_processors.h>

static constexpr auto numBridgeChans = 8;

#if (MSVC)
    #include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor

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

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Create state
    juce::ValueTree createEmptyOSCState();

    juce::ValueTree oscBridgeState;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState mPublicState { *this, nullptr, "Parameters", createParameterLayout() };

    void updateListenerStates();
    void setStateChangeCallbacks();

    auto& getChannel (auto index) { return mOscBridgeChannels[index]; }

private:
    std::vector<std::shared_ptr<OSCBridgeChannel>> mOscBridgeChannels;
    std::shared_ptr<OSCBridgeManager> mOscBridgeManager;

    std::vector<std::shared_ptr<LambdaStateListener>> mChanListeners {};
    std::shared_ptr<LambdaStateListener> mGlobalStateListener;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
