#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);

    // Set default look and feel
    lookAndFeel = std::make_unique<BirdHouse::BirdHouseLookAndFeel>();
    juce::LookAndFeel::setDefaultLookAndFeel (lookAndFeel.get());

    auto defaultFontSize = 24.0f;

    // Link
    hyperlinkButton.setButtonText ("(?)");
    hyperlinkButton.setFont (juce::Font (defaultFontSize, juce::Font::plain), false);
    hyperlinkButton.setColour (juce::HyperlinkButton::textColourId, BirdHouse::Colours::magenta);
    hyperlinkButton.setJustificationType (juce::Justification::right);
    addAndMakeVisible (hyperlinkButton);

    // Labels
    oscBridgeChannelLabels = std::make_unique<OSCBridgeChannelLabels>();
    oscBridgeChannelLabels->setFont (juce::Font (defaultFontSize, juce::Font::bold));
    oscBridgeChannelLabels->setColour (BirdHouse::Colours::blue);
    addAndMakeVisible (*oscBridgeChannelLabels);

    // Set font for title
    auto font = juce::Font (defaultFontSize * 1.33f, juce::Font::bold);
    titleLabel.setFont (font);
    titleLabel.setText ("BirdHouse. OSC to MIDI. v" VERSION ".", juce::dontSendNotification);
    titleLabel.setColour (juce::Label::textColourId, BirdHouse::Colours::fg);
    titleLabel.setVisible (true);
    addAndMakeVisible (titleLabel);

    auto numChannels = 8;
    for (auto i = 0; i < numChannels; ++i)
    {
        oscBridgeChannelEditors.push_back (std::make_unique<OSCBridgeChannelEditor>());

        // Set default values for the editors
        oscBridgeChannelEditors.back()->setFont (juce::Font (defaultFontSize, juce::Font::plain));
        oscBridgeChannelEditors.back()->setRecvPort (8000);
        oscBridgeChannelEditors.back()->setPath ("/" + juce::String (i + 1) + "/value");
        oscBridgeChannelEditors.back()->setInputMin (0.f);
        oscBridgeChannelEditors.back()->setInputMax (1.f);
        oscBridgeChannelEditors.back()->setOutputMidiChannel (1);
        oscBridgeChannelEditors.back()->setOutputNum (i + 1);
        oscBridgeChannelEditors.back()->setOutputMsgType (0);

        addAndMakeVisible (*oscBridgeChannelEditors.back());
    }

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1000, 600);

    // Allow resizing
    setResizable (true, false);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
    // layout the positions of your child components here
    auto area = getLocalBounds();
    auto oscChannelHeight = area.getHeight() / (oscBridgeChannelEditors.size() + 3);

    // Add title
    auto titleSize = oscChannelHeight;
    titleLabel.setBounds (area.removeFromTop (titleSize));

    // Labels
    oscBridgeChannelLabels->setBounds (area.removeFromTop (oscChannelHeight));

    for (auto& oscBridgeChannelEditor : oscBridgeChannelEditors)
    {
        oscBridgeChannelEditor->setBounds (area.removeFromTop (oscChannelHeight));
    }

    // Add link, place at top right
    // auto hyperLinkBounds = juce::Rectangle<int> (area.getRight() - hyperlinkWidth, 0, hyperlinkWidth, titleSize);
    auto hyperLinkBounds = area.removeFromTop (titleSize);
    hyperlinkButton.setBounds (hyperLinkBounds);
}
