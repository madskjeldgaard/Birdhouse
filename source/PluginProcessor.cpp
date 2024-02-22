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
    )

{
    // Set the version number
    parameters.state.setProperty ("BirdHouseVersion", VERSION, nullptr);

    // Add parameter listeners
    birdhouse::BirdHouseParams<numBridgeChans>::addParameterListeners (parameters, *this);

    // Construct and initialise all channels
    for (auto i = 0; i < numBridgeChans; ++i)
    {
        // Default values, these will be changed as soon as the state is loaded
        auto path = juce::String ("/" + juce::String (i + 1) + "/value");
        auto inMin = 0.0f;
        auto inMax = 1.0f;
        auto outChan = i + 1;
        auto outNum = 48 + i;
        auto msgType = 0;
        auto muted = false;

        // Set up the channel
        mOscBridgeChannels.push_back (std::make_shared<birdhouse::OSCBridgeChannel> (
            path, inMin, inMax, outChan, outNum, static_cast<birdhouse::MsgType> (msgType)));

        // Set mute
        mOscBridgeChannels[static_cast<std::size_t> (i)]->state().setMuted (muted);
    }

    // auto globalState = parameters.state;
    // mGlobalStateListener = std::make_shared<LambdaStateListener> (globalState);

    // setStateChangeCallbacks();
    // updateListenerStates();

    // Register all channels with the OSCBridge manager
    mOscBridgeManager = std::make_shared<birdhouse::OSCBridgeManager> (mOscBridgeChannels);
}

PluginProcessor::~PluginProcessor()
{
    // Remove listeners
    birdhouse::BirdHouseParams<numBridgeChans>::removeParameterListeners (parameters, *this);
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
    auto defaultPort = 6666;
    mOscBridgeManager->stopListening();
    auto connectionResult = mOscBridgeManager->startListening (defaultPort);
    juce::Logger::writeToLog ("Connection result: " + juce::String (static_cast<int> (connectionResult)));
    if (static_cast<bool> (connectionResult))
    {
        juce::Logger::writeToLog ("Connected to port " + juce::String (defaultPort));
    }
    else
    {
        juce::Logger::writeToLog ("Failed to connect to port " + juce::String (defaultPort));
    }
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    mOscBridgeManager->stopListening();

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

    if (mParametersNeedUpdating)
    {
        updateChannelsFromParams();
        mParametersNeedUpdating = false;
    }

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
    // std::unique_ptr<juce::XmlElement> xml (oscBridgeState.createXml());
    // copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    // if (xmlState.get() != nullptr)
    // {
    //     if (xmlState->hasTagName (oscBridgeState.getType()))
    //     {
    //         auto stateFromXML = juce::ValueTree::fromXml (*xmlState);
    //         if (stateFromXML.isValid())
    //         {
    //             oscBridgeState = juce::ValueTree::fromXml (*xmlState);
    //             updateListenerStates();
    //         }
    //     }
    // }
    // else
    // {
    //     juce::Logger::writeToLog ("OSC State is null");
    // }
}

// juce::ValueTree PluginProcessor::createEmptyOSCState()
// {
//     juce::ValueTree state ("OSCState");

//     // Add global settings
//     juce::ValueTree globalSettings ("GlobalSettings"),
//         channelSettings ("ChannelSettings");

//     // In the global settings, we can add the following:
//     // - Port
//     // - Version
//     //  - ConnectionStatus
//     // globalSettings.setProperty ("Port", 6666, nullptr);
//     globalSettings.setProperty ("ConnectionStatus", false, nullptr);
//     globalSettings.setProperty ("Version", VERSION, nullptr);

//     constexpr auto numChannels = 8;

//     // Each channel has it's own node tree
//     for (auto i = 0; i < numChannels; ++i)
//     {
//         // Channel state:
//         // Path
//         // inputMin
//         // inputMax
//         // outputMidiChannel
//         // outputMidiNum
//         // msgType
//         // muted
//         juce::ValueTree channel ("Channel");
//         channel.setProperty ("Path", "/" + juce::String (i) + "/value", nullptr);
//         channel.setProperty ("InputMin", 0.0f, nullptr);
//         channel.setProperty ("InputMax", 1.0f, nullptr);
//         channel.setProperty ("OutputMidiChannel", 1, nullptr);
//         channel.setProperty ("OutputMidiNum", i + 48, nullptr);
//         channel.setProperty ("MsgType", 0, nullptr);
//         channel.setProperty ("Muted", false, nullptr);

//         channelSettings.addChild (channel, -1, nullptr);
//     }

//     state.addChild (globalSettings, -1, nullptr);
//     state.addChild (channelSettings, -1, nullptr);

//     return state;
// }

// void PluginProcessor::updateListenerStates()
// {
//     // mGlobalStateListener->setState (oscBridgeState.getChildWithName ("GlobalSettings"));

//     // auto chanNum = 0;
//     // for (auto& listener : mChanListeners)
//     // {
//     //     listener->setState (oscBridgeState.getChildWithName ("ChannelSettings").getChild (chanNum++));
//     // }
// }

// void PluginProcessor::setStateChangeCallbacks()
// {
//     // Set up channel listeners
//     // auto i = 0u;
//     // for (auto& chanListener : mChanListeners)
//     // {
//     //     // Set defualt callback for state changes in properties
//     //     chanListener->setChangedCallback ([this, i] (auto whatChanged) {
//     //         auto chanState = oscBridgeState.getChildWithName ("ChannelSettings").getChild (static_cast<int> (i));

//     //         if (whatChanged == juce::Identifier ("Path"))
//     //         {
//     //             auto newPath = chanState.getProperty ("Path");
//     //             mOscBridgeChannels[i]->state().setPath (newPath);
//     //         }
//     //     });

//     //     i++;
//     // }

//     // Global state
//     // mGlobalStateListener->setChangedCallback ([this] (auto whatChanged) {
//     //     auto globalSettings = oscBridgeState.getChildWithName ("GlobalSettings");

//     //     if (globalSettings.isValid())
//     //     {
//     //         if (whatChanged == juce::Identifier ("Port"))
//     //         {
//     //             auto newPort = globalSettings.getProperty ("Port", 8000);
//     //             juce::Logger::writeToLog ("Changing port");

//     //             mOscBridgeManager->stopListening();
//     //             auto connectionResult = mOscBridgeManager->startListening (newPort);

//     //             globalSettings.setProperty ("ConnectionStatus", connectionResult, nullptr);
//     //         }

//     //         if (whatChanged == juce::Identifier ("ConnectionStatus"))
//     //         {
//     //             auto newStatus = globalSettings.getProperty ("ConnectionStatus", false);
//     //             juce::Logger::writeToLog ("Connection status changed to " + juce::String (newStatus));
//     //         }
//     //     }
//     //     else
//     //     {
//     //         juce::Logger::writeToLog ("Global settings not valid");
//     //     }
//     // });
// }

// Get all parameters from the apvts and update the channels accordingly
void PluginProcessor::updateChannelsFromParams()
{
    juce::Logger::writeToLog ("Updating channels from parameters");
    for (auto chanNum = 1; chanNum <= numBridgeChans; chanNum++)
    {
        // Identifiers
        const auto inMinParamID = juce::String ("InMin") + juce::String (chanNum);
        const auto inMaxParamID = juce::String ("InMax") + juce::String (chanNum);
        const auto midiChanParamID = juce::String ("MidiChan") + juce::String (chanNum);
        const auto midiNumParamID = juce::String ("MidiNum") + juce::String (chanNum);
        const auto msgTypeParamID = juce::String ("MsgType") + juce::String (chanNum);
        const auto mutedParamID = juce::String ("Muted") + juce::String (chanNum);

        // Params
        const auto inMinParam = parameters.getParameter (inMinParamID);
        const auto inMaxParam = parameters.getParameter (inMaxParamID);
        const auto midiChanParam = parameters.getParameter (midiChanParamID);
        const auto midiNumParam = parameters.getParameter (midiNumParamID);
        const auto msgTypeParam = parameters.getParameter (msgTypeParamID);
        const auto mutedParam = parameters.getParameter (mutedParamID);

        // Param Values
        const auto inMin = static_cast<juce::AudioParameterFloat*> (inMinParam)->get();
        const auto inMax = static_cast<juce::AudioParameterFloat*> (inMaxParam)->get();
        const auto midiChan = static_cast<juce::AudioParameterInt*> (midiChanParam)->get();
        const auto midiNum = static_cast<juce::AudioParameterInt*> (midiNumParam)->get();
        const auto msgType = static_cast<juce::AudioParameterInt*> (msgTypeParam)->get();
        const auto muted = static_cast<juce::AudioParameterInt*> (mutedParam)->get();

        const auto chan = mOscBridgeChannels[static_cast<std::size_t> (chanNum - 1)];
        chan->state().setInputMin (inMin);
        chan->state().setInputMax (inMax);
        chan->state().setOutputMidiChannel (static_cast<int> (midiChan));
        chan->state().setOutputMidiNum (static_cast<int> (midiNum));
        chan->state().setOutputType (static_cast<birdhouse::MsgType> (static_cast<int> (msgType)));
        chan->state().setMuted (static_cast<bool> (muted));
    }
}

void PluginProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    juce::Logger::writeToLog ("Parameter changed: " + parameterID + " = " + juce::String (newValue));
    mParametersNeedUpdating = true;
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
