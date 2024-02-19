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
    for (auto i = 0; i < numBridgeChans; ++i)
    {
        auto chanState = oscBridgeState.getChildWithName ("ChannelSettings").getChild (i);
        mChanListeners.emplace_back (std::make_shared<LambdaStateListener> (chanState));

        // Set up the channel
        mOscBridgeChannels.push_back (std::make_shared<OSCBridgeChannel> (chanState));
    }

    auto globalState = oscBridgeState.getChildWithName ("GlobalSettings");
    mGlobalStateListener = std::make_shared<LambdaStateListener> (globalState);

    setStateChangeCallbacks();

    // Register all channels with the OSCBridge manager
    mOscBridgeManager = std::make_shared<OSCBridgeManager>();
    auto chanNum = 0u;
    for (auto& chan : mOscBridgeChannels)
    {
        // Register the channel with the manager
        mOscBridgeManager->registerChannel (chan);

        // Add a custom callback to each channel
        // mOscBridgeManager->addCallbackToChannel (chanNum++, [&] (auto normalizedValue, auto valueAccepted, auto rawOSCMessage) {
        //     juce::ignoreUnused (normalizedValue, valueAccepted, rawOSCMessage);
        //     juce::Logger::writeToLog ("Normalized value: " + juce::String (normalizedValue) + " Value accepted: " + juce::String (static_cast<int> (valueAccepted)) + " Raw OSC message: " + rawOSCMessage.getAddressPattern().toString());

        // });
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

    auto port = oscBridgeState.getChildWithName ("GlobalSettings").getProperty ("Port");
    auto connectionResult = mOscBridgeManager->startListening (port);
    oscBridgeState.getChildWithName ("GlobalSettings").setProperty ("ConnectionStatus", connectionResult, nullptr);
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

    for (auto& chan : mOscBridgeChannels)
    {
        chan->appendMessagesTo (midiMessages);
    }

    buffer.clear();
    // Generate noise
    // -100 db in ampltiude
    // auto level = 0.001f;
    // noiseGen.fillBufferWithNoise (buffer, level);
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
    // TODO: Ensure state is valid before saving
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
            oscBridgeState = juce::ValueTree::fromXml (*xmlState);
            mPublicState.replaceState (oscBridgeState);

            updateListenerStates();
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
        channel.setProperty ("OutputMidiNum", i + 1, nullptr);
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

            juce::Logger::writeToLog ("Settings changed in channel" + juce::String (i) + " " + whatChanged.toString() + " " + chanState.getProperty (whatChanged).toString());
            if (whatChanged == juce::Identifier ("Path"))
            {
                auto newPath = chanState.getProperty ("Path");
                mOscBridgeChannels[i]->setPath (newPath);
            }

            if (whatChanged == juce::Identifier ("InputMin"))
            {
                auto newMin = chanState.getProperty ("InputMin");
                mOscBridgeChannels[i]->setInputMin (newMin);
            }

            if (whatChanged == juce::Identifier ("InputMax"))
            {
                auto newMax = chanState.getProperty ("InputMax");
                mOscBridgeChannels[i]->setInputMax (newMax);
            }

            if (whatChanged == juce::Identifier ("OutputMidiChannel"))
            {
                auto newChannel = chanState.getProperty ("OutputMidiChannel");
                mOscBridgeChannels[i]->setOutputMidiChannel (newChannel);
            }

            if (whatChanged == juce::Identifier ("OutputMidiNum"))
            {
                auto newNum = chanState.getProperty ("OutputMidiNum");
                mOscBridgeChannels[i]->setOutputMidiNum (static_cast<int> (newNum));
            }

            if (whatChanged == juce::Identifier ("MsgType"))
            {
                auto newType = chanState.getProperty ("MsgType");
                mOscBridgeChannels[i]->setOutputType (static_cast<OSCBridgeChannel::MsgType> (static_cast<int> (newType)));
            }

            if (whatChanged == juce::Identifier ("Muted"))
            {
                auto newMuted = chanState.getProperty ("Muted");
                mOscBridgeChannels[i]->setMuted (newMuted);
            }
        });

        i++;
    }

    // Global state
    mGlobalStateListener->setChangedCallback ([this] (auto whatChanged) {
        auto globalSettings = oscBridgeState.getChildWithName ("GlobalSettings");
        juce::Logger::writeToLog ("Global state changed: " + whatChanged.toString());

        if (whatChanged == juce::Identifier ("Port"))
        {
            auto newPort = globalSettings.getProperty ("Port");
            juce::Logger::writeToLog ("Port changed to " + juce::String (newPort));

            mOscBridgeManager->stopListening();
            auto connectionResult = mOscBridgeManager->startListening (newPort);

            globalSettings.setProperty ("ConnectionStatus", connectionResult, nullptr);
        }

        if (whatChanged == juce::Identifier ("ConnectionStatus"))
        {
            auto newStatus = globalSettings.getProperty ("ConnectionStatus");
            juce::Logger::writeToLog ("Connection status changed to " + juce::String (newStatus));
        }
    });
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
