#pragma once

#include "bridge/LambdaStateListener.h"
#include "bridge/OSCBridgeChannel.h"
#include "bridge/OSCBridgeManager.h"
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

    juce::ValueTree oscBridgeState { createEmptyOSCState() };

    void updateListenerStates();
    void setStateChangeCallbacks();

private:
    std::vector<std::shared_ptr<OSCBridgeChannel>> oscBridgeChannels;
    std::shared_ptr<OSCBridgeManager> oscBridgeManager;

    std::vector<std::shared_ptr<LambdaStateListener>> chanListeners {};
    std::shared_ptr<LambdaStateListener> globalStateListener;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
