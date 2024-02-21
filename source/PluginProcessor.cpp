#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
    : AudioProcessor (BusesProperties()
#if !JucePlugin_IsMidiEffect
    #if !JucePlugin_IsSynth
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
    #endif
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
            ),
      oscBridgeState (createEmptyOSCState())
{
    for (auto i = 0; i < numBridgeChans; ++i)
    {
        auto chanState = oscBridgeState.getChildWithName ("ChannelSettings").getChild (i);
        mChanListeners.emplace_back (std::make_shared<LambdaStateListener> (chanState));

        // Set up the channel
        auto path = chanState.getProperty ("Path", "/" + juce::String (i) + "/value");
        auto inMin = chanState.getProperty ("InputMin", 0.0f);
        auto inMax = chanState.getProperty ("InputMax", 1.0f);
        auto outChan = chanState.getProperty ("OutputMidiChannel", i + 1);
        auto outNum = chanState.getProperty ("OutputMidiNum", 48 + i);
        auto msgType = static_cast<birdhouse::MsgType> (static_cast<int> (chanState.getProperty ("MsgType", 0)));
        auto muted = chanState.getProperty ("Muted", false);
        mOscBridgeChannels.push_back (std::make_shared<birdhouse::OSCBridgeChannel> (
            path, inMin, inMax, outChan, outNum, msgType));

        // Set mute
        mOscBridgeChannels[static_cast<std::size_t>(i)]->state().setMuted (muted);
    }

    auto globalState = oscBridgeState.getChildWithName ("GlobalSettings");
    mGlobalStateListener = std::make_shared<LambdaStateListener> (globalState);

    setStateChangeCallbacks();
    updateListenerStates();

    // Register all channels with the OSCBridge manager
    mOscBridgeManager = std::make_shared<birdhouse::OSCBridgeManager>();
    for (auto& chan : mOscBridgeChannels)
    {
        // Register the channel with the manager
        mOscBridgeManager->registerChannel (chan);

        // Add a custom callback to each channel
        chan->addOSCCallback ([&] (auto normalizedValue, auto valueAccepted, auto rawOSCMessage) {
            juce::ignoreUnused (normalizedValue, valueAccepted, rawOSCMessage);
            juce::Logger::writeToLog ("Normalized value: " + juce::String (normalizedValue) + " Value accepted: " + juce::String (static_cast<int> (valueAccepted)) + " Raw OSC message: " + rawOSCMessage.getAddressPattern().toString());
        });
    }
}

PluginProcessor::~PluginProcessor()
{
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
        // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);

    juce::Logger::writeToLog ("Preparing to play");

    // Set default values for each channel
    auto index = 0;
    for (auto& chan : mOscBridgeChannels)
    {
        auto inMin = oscBridgeState.getChildWithName ("ChannelSettings").getChild (index).getProperty ("InputMin", 0.0f);
        auto inMax = oscBridgeState.getChildWithName ("ChannelSettings").getChild (index).getProperty ("InputMax", 1.0f);
        auto outChan = oscBridgeState.getChildWithName ("ChannelSettings").getChild (index).getProperty ("OutputMidiChannel", index + 1);
        auto outNum = oscBridgeState.getChildWithName ("ChannelSettings").getChild (index).getProperty ("OutputMidiNum", 48 + index);
        auto msgType = oscBridgeState.getChildWithName ("ChannelSettings").getChild (index).getProperty ("MsgType", 0);
        auto muted = oscBridgeState.getChildWithName ("ChannelSettings").getChild (index).getProperty ("Muted", false);
        chan->state().setInputMin (inMin);
        chan->state().setInputMax (inMax);
        chan->state().setOutputMidiChannel (outChan);
        chan->state().setOutputMidiNum (outNum);
        chan->state().setOutputType (static_cast<birdhouse::MsgType> (static_cast<int> (msgType)));
        chan->state().setMuted (muted);

        index++;
    }

    // Try connecting to default port
    auto defaultPort = oscBridgeState.getChildWithName ("GlobalSettings").getProperty ("Port", 8000);
    auto connectionResult = mOscBridgeManager->startListening (defaultPort);
    oscBridgeState.getChildWithName ("GlobalSettings").setProperty ("ConnectionStatus", connectionResult, nullptr);
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    // mOscBridgeManager->stopListening();

    juce::Logger::writeToLog ("Releasing resources");
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
    #if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    #endif

    return true;
#endif
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // Create temporary buffer for MIDI messages to allow double-buffering
    juce::MidiBuffer tmpMidi;

    for (auto& chan : mOscBridgeChannels)
    {
        chan->appendMessagesTo (tmpMidi);
    }

    midiMessages.swapWith (tmpMidi);
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{ // Use generic gui for editor for now
    return new PluginEditor (*this);
    // return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    std::unique_ptr<juce::XmlElement> xml (oscBridgeState.createXml());
    copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName (oscBridgeState.getType()))
        {
            auto stateFromXML = juce::ValueTree::fromXml (*xmlState);
            if (stateFromXML.isValid())
            {
                oscBridgeState = juce::ValueTree::fromXml (*xmlState);
                updateListenerStates();
            }
        }
    }
    else
    {
        juce::Logger::writeToLog ("OSC State is null");
    }
}

juce::ValueTree PluginProcessor::createEmptyOSCState()
{
    juce::ValueTree state ("OSCState");

    // Add global settings
    juce::ValueTree globalSettings ("GlobalSettings"),
        channelSettings ("ChannelSettings");

    // In the global settings, we can add the following:
    // - Port
    // - Version
    //  - ConnectionStatus
    globalSettings.setProperty ("Port", 6666, nullptr);
    globalSettings.setProperty ("ConnectionStatus", false, nullptr);
    globalSettings.setProperty ("Version", VERSION, nullptr);

    constexpr auto numChannels = 8;

    // Each channel has it's own node tree
    for (auto i = 0; i < numChannels; ++i)
    {
        // Channel state:
        // Path
        // inputMin
        // inputMax
        // outputMidiChannel
        // outputMidiNum
        // msgType
        // muted
        juce::ValueTree channel ("Channel");
        channel.setProperty ("Path", "/" + juce::String (i) + "/value", nullptr);
        channel.setProperty ("InputMin", 0.0f, nullptr);
        channel.setProperty ("InputMax", 1.0f, nullptr);
        channel.setProperty ("OutputMidiChannel", 1, nullptr);
        channel.setProperty ("OutputMidiNum", i + 48, nullptr);
        channel.setProperty ("MsgType", 0, nullptr);
        channel.setProperty ("Muted", false, nullptr);

        channelSettings.addChild (channel, -1, nullptr);
    }

    state.addChild (globalSettings, -1, nullptr);
    state.addChild (channelSettings, -1, nullptr);

    return state;
}

void PluginProcessor::updateListenerStates()
{
    mGlobalStateListener->setState (oscBridgeState.getChildWithName ("GlobalSettings"));

    auto chanNum = 0;
    for (auto& listener : mChanListeners)
    {
        listener->setState (oscBridgeState.getChildWithName ("ChannelSettings").getChild (chanNum++));
    }
}

void PluginProcessor::setStateChangeCallbacks()
{
    // Set up channel listeners
    auto i = 0u;
    for (auto& chanListener : mChanListeners)
    {
        // Set defualt callback for state changes in properties
        chanListener->setChangedCallback ([this, i] (auto whatChanged) {
            auto chanState = oscBridgeState.getChildWithName ("ChannelSettings").getChild (static_cast<int> (i));

            if (whatChanged == juce::Identifier ("Path"))
            {
                auto newPath = chanState.getProperty ("Path");
                mOscBridgeChannels[i]->state().setPath (newPath);
            }

            if (whatChanged == juce::Identifier ("InputMin"))
            {
                auto newMin = chanState.getProperty ("InputMin");
                mOscBridgeChannels[i]->state().setInputMin (newMin);
            }

            if (whatChanged == juce::Identifier ("InputMax"))
            {
                auto newMax = chanState.getProperty ("InputMax");
                mOscBridgeChannels[i]->state().setInputMax (newMax);
            }

            if (whatChanged == juce::Identifier ("OutputMidiChannel"))
            {
                auto newChannel = chanState.getProperty ("OutputMidiChannel");
                mOscBridgeChannels[i]->state().setOutputMidiChannel (newChannel);
            }

            if (whatChanged == juce::Identifier ("OutputMidiNum"))
            {
                auto newNum = chanState.getProperty ("OutputMidiNum");
                mOscBridgeChannels[i]->state().setOutputMidiNum (static_cast<int> (newNum));
            }

            if (whatChanged == juce::Identifier ("MsgType"))
            {
                auto newType = chanState.getProperty ("MsgType");
                mOscBridgeChannels[i]->state().setOutputType (static_cast<birdhouse::MsgType> (static_cast<int> (newType)));
            }

            if (whatChanged == juce::Identifier ("Muted"))
            {
                auto newMuted = chanState.getProperty ("Muted");
                mOscBridgeChannels[i]->state().setMuted (newMuted);
            }
        });

        i++;
    }

    // Global state
    mGlobalStateListener->setChangedCallback ([this] (auto whatChanged) {
        auto globalSettings = oscBridgeState.getChildWithName ("GlobalSettings");

        if (globalSettings.isValid())
        {
            if (whatChanged == juce::Identifier ("Port"))
            {
                auto newPort = globalSettings.getProperty ("Port", 8000);
                juce::Logger::writeToLog ("Changing port");

                mOscBridgeManager->stopListening();
                auto connectionResult = mOscBridgeManager->startListening (newPort);

                globalSettings.setProperty ("ConnectionStatus", connectionResult, nullptr);
            }

            if (whatChanged == juce::Identifier ("ConnectionStatus"))
            {
                auto newStatus = globalSettings.getProperty ("ConnectionStatus", false);
                juce::Logger::writeToLog ("Connection status changed to " + juce::String (newStatus));
            }
        }
        else
        {
            juce::Logger::writeToLog ("Global settings not valid");
        }
    });
}

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    // Add port parameter
    juce::AudioProcessorValueTreeState::ParameterLayout portLayout;

    auto maxPort = 65535;
    portLayout.add (std::make_unique<juce::AudioParameterInt> ("Port", "Port", 0, maxPort, 8000));

    for (auto chanNum = 1; chanNum <= numBridgeChans; chanNum++)
    {
        // Add InMin parameter for this channel
        auto inMinParamID = juce::String ("InMin") + juce::String (chanNum);
        auto inMinParameterName = juce::String ("InMin") + juce::String (chanNum);
        auto inMinMinValue = 0.f;
        auto inMinMaxValue = 20000.f;
        auto inMinDefaultValue = 0.f;
        portLayout.add (std::make_unique<juce::AudioParameterFloat> (inMinParamID, inMinParameterName, inMinMinValue, inMinMaxValue, inMinDefaultValue));

        // Add InMax parameter
        auto inMaxParamID = juce::String ("InMax") + juce::String (chanNum);
        auto inMaxParameterName = juce::String ("InMax") + juce::String (chanNum);
        auto inMaxMinValue = 0.f;
        auto inMaxMaxValue = 20000.f;
        auto inMaxDefaultValue = 1.f;
        portLayout.add (std::make_unique<juce::AudioParameterFloat> (inMaxParamID, inMaxParameterName, inMaxMinValue, inMaxMaxValue, inMaxDefaultValue));

        // Add MIDI Chan parameter
        auto midiChanParamID = juce::String ("MidiChan") + juce::String (chanNum);
        auto midiChanParameterName = juce::String ("MidiChan") + juce::String (chanNum);
        auto midiChanMinValue = 1;
        auto midiChanMaxValue = 16;
        auto midiChanDefaultValue = 1;
        portLayout.add (std::make_unique<juce::AudioParameterInt> (midiChanParamID, midiChanParameterName, midiChanMinValue, midiChanMaxValue, midiChanDefaultValue));

        // Add MIDI Num parameter
        auto midiNumParamID = juce::String ("MidiNum") + juce::String (chanNum);
        auto midiNumParameterName = juce::String ("MidiNum") + juce::String (chanNum);
        auto midiNumMinValue = 0;
        auto midiNumMaxValue = 127;
        auto midiNumDefaultValue = 48 + chanNum - 1;
        portLayout.add (std::make_unique<juce::AudioParameterInt> (midiNumParamID, midiNumParameterName, midiNumMinValue, midiNumMaxValue, midiNumDefaultValue));

        // Add MsgType parameter
        auto msgTypeParamID = juce::String ("MsgType") + juce::String (chanNum);
        auto msgTypeParameterName = juce::String ("MsgType") + juce::String (chanNum);
        auto msgTypeMinValue = 0;
        auto msgTypeMaxValue = birdhouse::MsgType::NumMsgTypes - 1;
        ;
        auto msgTypeDefaultValue = 0;
        portLayout.add (std::make_unique<juce::AudioParameterInt> (msgTypeParamID, msgTypeParameterName, msgTypeMinValue, msgTypeMaxValue, msgTypeDefaultValue));

        // Add Muted parameter
        auto mutedParamID = juce::String ("Muted") + juce::String (chanNum);
        auto mutedParameterName = juce::String ("Muted") + juce::String (chanNum);
        auto mutedMinValue = 0;
        auto mutedMaxValue = 1;
        auto mutedDefaultValue = 0;
        portLayout.add (std::make_unique<juce::AudioParameterInt> (mutedParamID, mutedParameterName, mutedMinValue, mutedMaxValue, mutedDefaultValue));
    }

    return portLayout;
}

// Get all parameters from the apvts and update the channels accordingly
void PluginProcessor::updateChannelsFromParams()
{
    for (auto chanNum = 1; chanNum <= numBridgeChans; chanNum++)
    {
        const auto inMinParamID = juce::String ("InMin") + juce::String (chanNum);
        const auto inMaxParamID = juce::String ("InMax") + juce::String (chanNum);
        const auto midiChanParamID = juce::String ("MidiChan") + juce::String (chanNum);
        const auto midiNumParamID = juce::String ("MidiNum") + juce::String (chanNum);
        const auto msgTypeParamID = juce::String ("MsgType") + juce::String (chanNum);
        const auto mutedParamID = juce::String ("Muted") + juce::String (chanNum);

        const auto inMin = parameters.getParameter (inMinParamID)->getValue();
        const auto inMax = parameters.getParameter (inMaxParamID)->getValue();
        const auto midiChan = parameters.getParameter (midiChanParamID)->getValue();
        const auto midiNum = parameters.getParameter (midiNumParamID)->getValue();
        const auto msgType = parameters.getParameter (msgTypeParamID)->getValue();
        const auto muted = parameters.getParameter (mutedParamID)->getValue();

        const auto chan = mOscBridgeChannels[static_cast<std::size_t> (chanNum - 1)];
        chan->state().setInputMin (inMin);
        chan->state().setInputMax (inMax);
        chan->state().setOutputMidiChannel (static_cast<int> (midiChan));
        chan->state().setOutputMidiNum (static_cast<int> (midiNum));
        chan->state().setOutputType (static_cast<birdhouse::MsgType> (static_cast<int> (msgType)));
        chan->state().setMuted (static_cast<bool> (muted));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
