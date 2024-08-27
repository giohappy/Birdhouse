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
    DBG ("Constructing PluginProcessor");
    // Set the version number
    parameters.state.setProperty ("BirdHouseVersion", VERSION, nullptr);

    // Add parameter listeners
    birdhouse::BirdHouseParams::addParameterListeners (parameters, *this);

    // Default values, these will be changed as soon as the state is loaded
    auto defaultPath = juce::String ("/value");
    auto path = parameters.state.getProperty ("Path", defaultPath);
    auto inMin = parameters.state.getProperty ("InMin", 0.0f);
    auto inMax = parameters.state.getProperty ("InMax", 1.0f);
    auto outChan = parameters.state.getProperty ("MidiChan", 1);
    auto outNum = parameters.state.getProperty ("MidiNum", 48);
    auto msgType = parameters.state.getProperty ("MsgType", 1);
    auto muted = parameters.state.getProperty ("Muted", false);

    // Set up the channel
    //mOscBridgeChannels.push_back (std::make_shared<birdhouse::OSCBridgeChannel> (
    //    path, inMin, inMax, outChan, outNum, static_cast<birdhouse::MsgType> (static_cast<int> (msgType))));

    mOscBridgeChannel = std::make_shared<birdhouse::OSCBridgeChannel> (
        path, inMin, inMax, outChan, outNum, static_cast<birdhouse::MsgType> (static_cast<int> (msgType)));

    DBG ("Initial state of channel is :");
    DBG ("Path: " + path.toString());
    DBG ("InMin: " + inMin.toString());
    DBG ("InMax: " + inMax.toString());
    DBG ("OutChan: " + outChan.toString());
    DBG ("OutNum: " + outNum.toString());
    DBG ("MsgType: " + msgType.toString());
    DBG ("Muted: " + muted.toString());

    // Set mute
    //mOscBridgeChannels[index]->state().setMuted (muted);

    mOscBridgeChannel->state().setMuted (muted);

    // Register all channels with the OSCBridge manager
    //mOscBridgeManager = std::make_shared<birdhouse::OSCBridgeManager> (mOscBridgeChannels);

    mOscBridgeManager = std::make_shared<birdhouse::OSCBridgeManager> (mOscBridgeChannel);

    // Set up listeners for the state changes
    mGlobalStateListener = std::make_shared<LambdaStateListener> (parameters.state);

    setStateChangeCallbacks();
    updateListenerStates();
}

PluginProcessor::~PluginProcessor()
{
    // Remove listeners
    birdhouse::BirdHouseParams::removeParameterListeners (parameters, *this);
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

        if (mOscBridgeChannel->state().midiChanged())
        {
            tmpMidi.addEvent (juce::MidiMessage::allNotesOff (mOscBridgeChannel->state().outChan()), sampleNum);
            mOscBridgeChannel->state().resetMidiFlag();
        }
        mOscBridgeChannel->appendMessagesTo (tmpMidi, sampleNum);
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
            updateValuesFromNonAudioParams (parameters.state);
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
            auto newStatus = state.getProperty ("ConnectionStatus", fallbackValue).toString();
            juce::Logger::writeToLog ("Connection status changed to " + newStatus);
            return;
        }

        // Update the non-audio parameters from the state
        updateValuesFromNonAudioParams (state);
    });
}

// Update the non-audio parameters from the state
// These are the parameters not directly exposed the the plugin host as parameters, like path, in min / max, etc.
void PluginProcessor::updateValuesFromNonAudioParams (auto state)
{
    const auto pathIdentifier = juce::Identifier (juce::String ("Path"));
    const auto inMinIdentifier = juce::Identifier (juce::String ("InMin"));
    const auto inMaxIdentifier = juce::Identifier (juce::String ("InMax"));

    auto pathFallbackValue = juce::String ("/value");
    auto newPath = state.getProperty (pathIdentifier, pathFallbackValue).toString();
    auto newInMin = state.getProperty (inMinIdentifier, 0.0f);
    auto newInMax = state.getProperty (inMaxIdentifier, 1.0f);

    mOscBridgeChannel->state().setPath (newPath);
    mOscBridgeChannel->state().setInputMin (newInMin);
    mOscBridgeChannel->state().setInputMax (newInMax);
}

// Update internal state from audio parameters.
// This gets all parameters from the apvts and updates the channels accordingly.
void PluginProcessor::updateChannelsFromParams()
{
    DBG ("Updating channels from parameters");
    // Identifiers
    const auto midiChanParamID = juce::String ("MidiChan");
    const auto midiNumParamID = juce::String ("MidiNum");
    const auto msgTypeParamID = juce::String ("MsgType");
    const auto mutedParamID = juce::String ("Muted");

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

    const auto chan = mOscBridgeChannel;
    chan->state().setOutputMidiChannel (static_cast<int> (midiChan));
    chan->state().setOutputMidiNum (static_cast<int> (midiNum));
    const auto msgTypeval = static_cast<birdhouse::MsgType> (static_cast<int> (msgType));
    DBG (" Setting output msgtype to " + juce::String (msgTypeval) + " with value " + juce::String (msgType));
    chan->state().setOutputType (msgTypeval);
    chan->state().setMuted (static_cast<bool> (muted));
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
