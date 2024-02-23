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

    // Register all channels with the OSCBridge manager
    mOscBridgeManager = std::make_shared<birdhouse::OSCBridgeManager> (mOscBridgeChannels);

    // Set up listeners for the state changes
    mGlobalStateListener = std::make_shared<LambdaStateListener> (parameters.state);

    setStateChangeCallbacks();
    updateListenerStates();
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

    // Set default values for each channel
    // if (!isConnected())
    // {
    auto defaultPort = parameters.getParameter ("Port");
    auto defaultPortVal = static_cast<juce::AudioParameterInt*> (defaultPort)->get();
    tryConnect (defaultPortVal);
    // }
}

void PluginProcessor::tryConnect (auto port)
{
    mOscBridgeManager->stopListening();
    auto connectionResult = mOscBridgeManager->startListening (static_cast<int> (port));
    DBG ("Connection result: " + juce::String (static_cast<int> (connectionResult)));
    if (static_cast<bool> (connectionResult))
    {
        DBG ("Connected to port " + juce::String (port));
    }
    else
    {
        DBG ("Failed to connect to port " + juce::String (port));
    }

    mConnected = static_cast<bool> (connectionResult);
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    mOscBridgeManager->stopListening();
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

    for (auto sampleNum = 0; sampleNum < buffer.getNumSamples(); ++sampleNum)
    {
        // Check if any of the channels have had changes in midi, if so, append note off to all channels
        // This is to prevent stuck notes
        for (auto& chan : mOscBridgeChannels)
        {
            if (chan->state().midiChanged())
            {
                tmpMidi.addEvent (juce::MidiMessage::allNotesOff (chan->state().outChan()), sampleNum);
                chan->state().resetMidiFlag();
            }

            chan->appendMessagesTo (tmpMidi, sampleNum);
        }
    }

    // Replace the original midiMessages with the processed ones
    midiMessages.swapWith (tmpMidi);
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    DBG ("Getting state information from xml");
    copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName (parameters.state.getType()))
        {
            DBG ("Setting state from xml");
            parameters.state = juce::ValueTree::fromXml (*xmlState);
            updateListenerStates();
        }
    }
}

// Set up lambda listeners to listen for  changes in the global state.
// This is used to sync the values between gui and processor for parameters that are not registered as audio parameters
void PluginProcessor::updateListenerStates()
{
    mGlobalStateListener->setState (parameters.state);
}

void PluginProcessor::setStateChangeCallbacks()
{
    DBG ("Setting state change callbacks");
    // Global state
    mGlobalStateListener->setChangedCallback ([this] (auto whatChanged) {
        auto state = parameters.state;

        DBG ("State changed: " + whatChanged.toString() + " " + state.getType().toString());

        // if (whatChanged == juce::Identifier ("Port"))
        // {
        //     tryConnect(state.getProperty ("Port", 6666);
        //     return;
        // }

        if (whatChanged == juce::Identifier ("ConnectionStatus"))
        {
            auto fallbackValue = false;
            auto newStatus = state.getProperty ("ConnectionStatus", fallbackValue);
            juce::Logger::writeToLog ("Connection status changed to " + juce::String (newStatus));
            return;
        }

        // Iterate over all channels and check if their state has changed
        for (auto chanNum = 1u; chanNum <= numBridgeChans; chanNum++)
        {
            const auto pathIdentifier = juce::Identifier (juce::String ("Path") + juce::String (chanNum));
            const auto inMinIdentifier = juce::Identifier (juce::String ("InMin") + juce::String (chanNum));
            const auto inMaxIdentifier = juce::Identifier (juce::String ("InMax") + juce::String (chanNum));

            if (whatChanged == pathIdentifier || whatChanged == inMinIdentifier || whatChanged == inMaxIdentifier)
            {
                DBG ("State changed: " + whatChanged.toString() + " " + state.getType().toString() + " chan" + juce::String (chanNum));
                auto pathFallbackValue = juce::String ("/" + juce::String (chanNum) + "/value");
                auto newPath = state.getProperty (pathIdentifier, pathFallbackValue);
                auto newInMin = state.getProperty (inMinIdentifier, 0.0f);
                auto newInMax = state.getProperty (inMaxIdentifier, 1.0f);

                mOscBridgeChannels[chanNum - 1]->state().setPath (newPath);
                mOscBridgeChannels[chanNum - 1]->state().setInputMin (newInMin);
                mOscBridgeChannels[chanNum - 1]->state().setInputMax (newInMax);
            }
        }
    });
}

// Update internal state from audio parameters.
// This gets all parameters from the apvts and updates the channels accordingly.
void PluginProcessor::updateChannelsFromParams()
{
    juce::Logger::writeToLog ("Updating channels from parameters");
    for (auto chanNum = 1; chanNum <= numBridgeChans; chanNum++)
    {
        // Identifiers
        const auto midiChanParamID = juce::String ("MidiChan") + juce::String (chanNum);
        const auto midiNumParamID = juce::String ("MidiNum") + juce::String (chanNum);
        const auto msgTypeParamID = juce::String ("MsgType") + juce::String (chanNum);
        const auto mutedParamID = juce::String ("Muted") + juce::String (chanNum);

        // Params
        const auto midiChanParam = parameters.getParameter (midiChanParamID);
        const auto midiNumParam = parameters.getParameter (midiNumParamID);
        const auto msgTypeParam = parameters.getParameter (msgTypeParamID);
        const auto mutedParam = parameters.getParameter (mutedParamID);

        // Param Values
        const auto midiChan = static_cast<juce::AudioParameterInt*> (midiChanParam)->get();
        const auto midiNum = static_cast<juce::AudioParameterInt*> (midiNumParam)->get();
        const auto msgType = static_cast<juce::AudioParameterInt*> (msgTypeParam)->get();
        const auto muted = static_cast<juce::AudioParameterBool*> (mutedParam)->get();

        const auto chan = mOscBridgeChannels[static_cast<std::size_t> (chanNum - 1)];
        chan->state().setOutputMidiChannel (static_cast<int> (midiChan));
        chan->state().setOutputMidiNum (static_cast<int> (midiNum));
        chan->state().setOutputType (static_cast<birdhouse::MsgType> (static_cast<int> (msgType)));
        chan->state().setMuted (static_cast<bool> (muted));
    }
}

void PluginProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    DBG ("Parameter changed: " + parameterID + " = " + juce::String (newValue));

    if (parameterID == "Port")
    {
        tryConnect (newValue);
    }
    mParametersNeedUpdating = true;
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
