#pragma once

#include "bridge/OSCBridgeChannel.h"
#include <juce_audio_processors/juce_audio_processors.h>

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

private:
    // std::unique_ptr<std::vector<OSCBridgeChannel>> oscBridgeChannels;
    OSCBridgeChannel oscBridgeChannel1 { 6666, "/note1", 0.0f, 1.0f, 1, 48, OSCBridgeChannel::MidiNote };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
