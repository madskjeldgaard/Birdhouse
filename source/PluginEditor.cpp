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
    auto labelFont = juce::Font (defaultFontSize, juce::Font::bold);
    auto labelColour = BirdHouse::Colours::blue;
    oscBridgeChannelLabels = std::make_unique<OSCBridgeChannelLabels>();
    oscBridgeChannelLabels->setFont (labelFont);
    oscBridgeChannelLabels->setColour (labelColour);
    addAndMakeVisible (*oscBridgeChannelLabels);

    // Set font for title
    auto titleFont = juce::Font (defaultFontSize * 1.33f, juce::Font::bold);
    titleLabel.setFont (titleFont);
    titleLabel.setText ("BirdHouse. An OSC to MIDI bridge. v" VERSION ".", juce::dontSendNotification);
    titleLabel.setColour (juce::Label::textColourId, BirdHouse::Colours::fg);
    titleLabel.setVisible (true);
    addAndMakeVisible (titleLabel);

    auto numChannels = 8;
    for (auto i = 0; i < numChannels; ++i)
    {
        oscBridgeChannelEditors.push_back (std::make_unique<OSCBridgeChannelEditor>());

        // Set default values for the editors
        oscBridgeChannelEditors.back()->setFont (juce::Font (defaultFontSize, juce::Font::plain));
        oscBridgeChannelEditors.back()->setPath ("/" + juce::String (i + 1) + "/value");
        oscBridgeChannelEditors.back()->setInputMin (0.f);
        oscBridgeChannelEditors.back()->setInputMax (1.f);
        oscBridgeChannelEditors.back()->setOutputMidiChannel (1);
        oscBridgeChannelEditors.back()->setOutputNum (i + 1);
        oscBridgeChannelEditors.back()->setOutputMsgType (0);

        addAndMakeVisible (*oscBridgeChannelEditors.back());
    }

    // Port
    portEditor.setFont (juce::Font (defaultFontSize, juce::Font::plain));
    portEditor.setJustification (juce::Justification::left);
    portEditor.setText (juce::String (8000));
    addAndMakeVisible (portEditor);

    // Port label
    portLabel.setFont (labelFont);
    portLabel.setColour (juce::Label::textColourId, labelColour);
    portLabel.setText ("Port", juce::dontSendNotification);
    addAndMakeVisible (portLabel);

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
    auto area = getLocalBounds();
    auto totalChannels = oscBridgeChannelEditors.size() + 5; // Including title, port, and hyperlink in count
    auto itemHeight = area.getHeight() / totalChannels;

    // Title area at the top
    titleLabel.setBounds (area.removeFromTop (itemHeight));

    // Adjusting for the channels' labels and editors
    oscBridgeChannelLabels->setBounds (area.removeFromTop (itemHeight));

    for (auto& oscBridgeChannelEditor : oscBridgeChannelEditors)
    {
        oscBridgeChannelEditor->setBounds (area.removeFromTop (itemHeight));
    }

    // Layout for portLabel, portEditor, and hyperlinkButton at the bottom
    // auto bottomArea = area.removeFromBottom (itemHeight); // Reserve space at the bottom
    auto bottomArea = area.removeFromBottom (itemHeight); // Reserve space at the bottom

    // Split the bottom area into three parts
    auto portLabelWidth = bottomArea.getWidth() * 0.1f; // Adjust the ratio as needed
    auto portEditorWidth = bottomArea.getWidth() * (1.0f / 7.0f); // 1/7th of the width
    auto hyperlinkWidth = bottomArea.getWidth() * 0.2f; // Adjust as needed

    // Place portLabel on the left side of the bottom area
    portLabel.setBounds (bottomArea.removeFromLeft (portLabelWidth));

    // Place portEditor next to portLabel
    portEditor.setBounds (bottomArea.removeFromLeft (portEditorWidth));

    // Place hyperlinkButton on the far right of the bottom area
    hyperlinkButton.setBounds (bottomArea.removeFromRight (hyperlinkWidth));
}
