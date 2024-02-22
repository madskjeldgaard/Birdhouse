#pragma once

#include "BinaryData.h"
#include "PluginProcessor.h"
#include "bridge/OSCActivityListener.h"
#include "gui/BirdHouseLookAndFeel.h"
#include "gui/OSCBridgeChannelEditor.h"
#include "gui/OSCBridgeChannelLabels.h"

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

    // Labels
    std::unique_ptr<BirdHouse::BirdHouseLookAndFeel> lookAndFeel;
    juce::Label titleLabel { "BirdHouse" }, portLabel { "Port" }, connectionStatusTitleLabel { "Connection Status" }, connectionStatusLabel { "Disconnected" };
    std::unique_ptr<OSCBridgeChannelLabels> oscBridgeChannelLabels;

    // Link to help / info
    juce::HyperlinkButton hyperlinkButton { "?", juce::URL { "https://github.com/madskjeldgaard/Birdhouse" } };

    // Port
    juce::TextEditor portEditor;
    // Parameter attachment
    std::unique_ptr<birdhouse::TextEditorAttachment<int>> portAttachment{
        std::make_unique<birdhouse::TextEditorAttachment<int>> (processorRef.parameters, portEditor, nullptr)
  };

    // GUI for each channel
    std::vector<std::unique_ptr<OSCBridgeChannelEditor>> oscBridgeChannelEditors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
