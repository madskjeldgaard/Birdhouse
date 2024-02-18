#pragma once

#include "BinaryData.h"
#include "BirdHouseLookAndFeel.h"
#include "PluginProcessor.h"
#include "bridge/OSCBridgeChannelEditor.h"
#include "bridge/OSCBridgeChannelLabels.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor& processorRef;

    juce::Label titleLabel { "BirdHouse" };

    juce::HyperlinkButton hyperlinkButton { "?", juce::URL { "https://github.com/madskjeldgaard/Birdhouse" } };
    std::unique_ptr<BirdHouse::BirdHouseLookAndFeel> lookAndFeel;


  juce::Label portLabel { "Port" };
    juce::TextEditor portEditor;
    std::vector<std::unique_ptr<OSCBridgeChannelEditor>> oscBridgeChannelEditors;

    std::unique_ptr<OSCBridgeChannelLabels> oscBridgeChannelLabels;

    // OSCBridgeChannelEditor oscBridgeChannelEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
