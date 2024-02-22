#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
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
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setVisible (true);
    addAndMakeVisible (titleLabel);

    // Get state
    // auto state = processorRef.oscBridgeState;

    // if (!state.isValid())
    // {
    //     juce::Logger::writeToLog ("State is invalid, using default");
    //     state = processorRef.createEmptyOSCState();
    // }

    // Set up each channel
    for (auto chanNum = 1u; chanNum <= numBridgeChans; chanNum++)
    {
        oscBridgeChannelEditors.push_back (std::make_unique<OSCBridgeChannelEditor> (chanNum, processorRef.parameters));

        // Set default values for the editors
        oscBridgeChannelEditors.back()->setFont (juce::Font (defaultFontSize, juce::Font::plain));
        oscBridgeChannelEditors.back()->setTitle (juce::String (chanNum));

        // Get defaults from the non-audio parameter parameters and set them in the GUI
        auto path = processorRef.parameters.state.getProperty ("Path" + juce::String (chanNum), "/" + juce::String (chanNum) + "/value");
        auto inMin = processorRef.parameters.state.getProperty ("InMin" + juce::String (chanNum), 0.0f);
        auto inMax = processorRef.parameters.state.getProperty ("InMax" + juce::String (chanNum), 1.0f);
        oscBridgeChannelEditors.back()->setPath (path);
        oscBridgeChannelEditors.back()->setInMin (inMin);
        oscBridgeChannelEditors.back()->setInMax (inMax);

        // Now set up the defaults for audio parameters
        auto midiChanParam = processorRef.parameters.getParameter ("MidiChan" + juce::String (chanNum));
        auto midiChan = static_cast<juce::AudioParameterInt*> (midiChanParam)->get();
        auto midiNumParam = processorRef.parameters.getParameter ("MidiNum" + juce::String (chanNum));
        auto midiNum = static_cast<juce::AudioParameterInt*> (midiNumParam)->get();
        auto msgTypeParam = processorRef.parameters.getParameter ("MsgType" + juce::String (chanNum));
        auto msgType = static_cast<juce::AudioParameterInt*> (msgTypeParam)->get();
        auto mutedParam = processorRef.parameters.getParameter ("Muted" + juce::String (chanNum));
        auto muted = static_cast<juce::AudioParameterBool*> (mutedParam)->get();

        oscBridgeChannelEditors.back()->setMidiChan (midiChan);
        oscBridgeChannelEditors.back()->setMidiNum (midiNum);
        oscBridgeChannelEditors.back()->setMsgType (msgType);
        oscBridgeChannelEditors.back()->setMuted (muted);

        addAndMakeVisible (*oscBridgeChannelEditors.back());
    }

    // Port
    // auto port = processorRef.parameters.state.getProperty ("Port", 8000);
    auto port = 1;
    portEditor.setFont (juce::Font (defaultFontSize, juce::Font::plain));
    portEditor.setJustification (juce::Justification::left);
    portEditor.setText (juce::String (port));
    portEditor.setInputRestrictions (5, "1234567890");

    // Filter text
    portEditor.onTextChange = [this] {
        auto newport = portEditor.getText().getIntValue();

        // Port can only be a number from 0 to 65535
        // If the port is not a number, set it to 8000
        // If the port is less than 0, set it to 0
        // If the port is greater than 65535, set it to 65535
        // Otherwise, set it to the new value
        if (newport < 0)
        {
            portEditor.setText ("0", false);
        }
        else if (newport > 65535)
        {
            portEditor.setText ("65535", false);
        }
    };
    portAttachment->attach ("Port");

    // On return -> lose focus
    // portEditor.onReturnKey = [this] {
    //     portEditor.focusLost (FocusChangeType::focusChangedDirectly);
    // };

    // // Focus lost -> update state
    // portEditor.onFocusLost = [this] {
    //     auto newport = portEditor.getText().getIntValue();
    //     auto globalSettings = processorRef.oscBridgeState.getChildWithName ("GlobalSettings");
    //     globalSettings.setProperty ("Port", newport, nullptr);
    // };

    addAndMakeVisible (portEditor);

    // Port label
    portLabel.setFont (labelFont);
    portLabel.setColour (juce::Label::textColourId, labelColour);
    portLabel.setText ("Port", juce::dontSendNotification);
    addAndMakeVisible (portLabel);

    // Connection status
    connectionStatusTitleLabel.setFont (labelFont);
    connectionStatusTitleLabel.setColour (juce::Label::textColourId, labelColour);
    connectionStatusTitleLabel.setText ("Status:", juce::dontSendNotification);
    connectionStatusTitleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (connectionStatusTitleLabel);

    connectionStatusLabel.setFont (labelFont);
    connectionStatusLabel.setColour (juce::Label::textColourId, BirdHouse::Colours::fg);
    connectionStatusLabel.setText ("Disconnected", juce::dontSendNotification);
    connectionStatusLabel.setJustificationType (juce::Justification::left);

    addAndMakeVisible (connectionStatusLabel);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1000, 500);

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

    // Update connection status
    // auto connectionStatus = processorRef.oscBridgeState.getChildWithName ("GlobalSettings").getProperty ("ConnectionStatus", false);

    // if (connectionStatus)
    // {
    //     // portEditor.applyColourToAllText (BirdHouse::Colours::green, true);
    //     connectionStatusLabel.setText ("Connected", juce::dontSendNotification);
    //     connectionStatusLabel.setColour (juce::Label::textColourId, BirdHouse::Colours::green);
    // }
    // else
    // {
    //     // portEditor.applyColourToAllText (BirdHouse::Colours::red, true);

    //     connectionStatusLabel.setColour (juce::Label::textColourId, BirdHouse::Colours::red);
    //     connectionStatusLabel.setText ("Disconnected", juce::dontSendNotification);
    // }

    // Update the activity indicators

    auto chanIndex = 0u;
    for (auto& oscBridgeChannelEditor : oscBridgeChannelEditors)
    {
        auto& chan = *processorRef.getChannel (chanIndex);

        // oscBridgeChannelEditor->updateActivityForChan (processorChan.state());

        chanIndex++;
    }
}

void PluginEditor::resized()
{
    auto area = getLocalBounds();
    auto totalChannels = oscBridgeChannelEditors.size() + 4; // Including title, port, and hyperlink in count
    auto itemHeight = static_cast<int> (static_cast<float> (area.getHeight()) / totalChannels);

    // Title area at the top
    titleLabel.setBounds (area.removeFromTop (itemHeight));

    // Adjusting for the channels' labels and editors
    oscBridgeChannelLabels->setBounds (area.removeFromTop (itemHeight));

    for (auto& oscBridgeChannelEditor : oscBridgeChannelEditors)
    {
        oscBridgeChannelEditor->setBounds (area.removeFromTop (itemHeight));
    }

    // Layout for portLabel, portEditor, and hyperlinkButton at the bottom
    auto bottomArea = area.removeFromBottom (itemHeight); // Reserve space at the bottom

    // Split the bottom area into three parts
    auto portLabelWidth = static_cast<int> (bottomArea.getWidth() * 0.1f); // Adjust the ratio as needed
    auto portEditorWidth = static_cast<int> (bottomArea.getWidth() * (1.0f / 6.0f)); // 1/7th of the width
    auto hyperlinkWidth = static_cast<int> (bottomArea.getWidth() * 0.2f); // Adjust as needed

    // Place portLabel on the left side of the bottom area
    portLabel.setBounds (bottomArea.removeFromLeft (portLabelWidth));

    // Place portEditor next to portLabel
    portEditor.setBounds (bottomArea.removeFromLeft (portEditorWidth));

    // Connection status
    connectionStatusTitleLabel.setBounds (bottomArea.removeFromLeft (portLabelWidth));
    connectionStatusLabel.setBounds (bottomArea.removeFromLeft (portEditorWidth));

    // Place hyperlinkButton on the far right of the bottom area
    hyperlinkButton.setBounds (bottomArea.removeFromRight (hyperlinkWidth));
}
