#pragma once

#include "../bridge/OSCBridgeChannel.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace birdhouse
{
    class BirdHouseParams
    {
    public:
        static void addParameterListeners (juce::AudioProcessorValueTreeState& state, auto& processor)
        {
            auto inMinParamID = juce::String ("InMin");
            auto inMaxParamID = juce::String ("InMax");
            auto midiChanParamID = juce::String ("MidiChan");
            auto midiNumParamID = juce::String ("MidiNum");
            auto msgTypeParamID = juce::String ("MsgType");
            auto mutedParamID = juce::String ("Muted");

            state.addParameterListener (inMinParamID, &processor);
            state.addParameterListener (inMaxParamID, &processor);
            state.addParameterListener (midiChanParamID, &processor);
            state.addParameterListener (midiNumParamID, &processor);
            state.addParameterListener (msgTypeParamID, &processor);
            state.addParameterListener (mutedParamID, &processor);

            // Global parameters:
            // Add listener to port parameter
            state.addParameterListener ("Port", &processor);
        }

        static void removeParameterListeners (juce::AudioProcessorValueTreeState& state, auto& processor)
        {
            // Remove listener from each parameter
            auto inMinParamID = juce::String ("InMin");
            auto inMaxParamID = juce::String ("InMax");
            auto midiChanParamID = juce::String ("MidiChan");
            auto midiNumParamID = juce::String ("MidiNum");
            auto msgTypeParamID = juce::String ("MsgType");
            auto mutedParamID = juce::String ("Muted");

            state.removeParameterListener (inMinParamID, &processor);
            state.removeParameterListener (inMaxParamID, &processor);
            state.removeParameterListener (midiChanParamID, &processor);
            state.removeParameterListener (midiNumParamID, &processor);
            state.removeParameterListener (msgTypeParamID, &processor);
            state.removeParameterListener (mutedParamID, &processor);

            // Global parameters:
            // Remove listener from port parameter
            state.removeParameterListener ("Port", &processor);
        }

        // This will set up all the non-Audio Parameter parameters (such as port, inMin, inMax) in the state of the AudioProcessorValueTreeState;
        static void setupNonAudioParameters (juce::AudioProcessorValueTreeState& state)
        {
            const auto inMinParamID = juce::Identifier (juce::String ("InMin"));
            const auto inMaxParamID = juce::Identifier (juce::String ("InMax"));
            const auto pathParamID = juce::Identifier (juce::String ("Path"));

            state.state.setProperty (inMinParamID, 0.0f, nullptr);
            state.state.setProperty (inMaxParamID, 1.0f, nullptr);
            state.state.setProperty (pathParamID, + "/value", nullptr);
        }

        static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
        {
            // Add port parameter
            juce::AudioProcessorValueTreeState::ParameterLayout portLayout;

            auto maxPort = 65535;
            portLayout.add (std::make_unique<juce::AudioParameterInt> ("Port", "Port", 0, maxPort, 8000));

            // Add InMin parameter for this channel
            // auto inMinParamID = juce::String ("InMin") + juce::String (chanNum);
            // auto inMinParameterName = juce::String ("InMin") + juce::String (chanNum);
            // auto inMinMinValue = 0.f;
            // auto inMinMaxValue = 20000.f;
            // auto inMinDefaultValue = 0.f;
            // auto inMinIntervalValue = 0.e1f;
            // auto normaliseableRange = juce::NormalisableRange<float> (inMinMinValue, inMinMaxValue);
            // portLayout.add (std::make_unique<juce::AudioParameterFloat> (inMinParamID, inMinParameterName, normaliseableRange, inMinDefaultValue));

            // Add InMax parameter
            // auto inMaxParamID = juce::String ("InMax") + juce::String (chanNum);
            // auto inMaxParameterName = juce::String ("InMax") + juce::String (chanNum);
            // auto inMaxMinValue = 0.f;
            // auto inMaxMaxValue = 20000.f;
            // auto inMaxDefaultValue = 1.f;
            // portLayout.add (std::make_unique<juce::AudioParameterFloat> (inMaxParamID, inMaxParameterName, inMaxMinValue, inMaxMaxValue, inMaxDefaultValue));

            // Add MIDI Chan parameter
            auto midiChanParamID = juce::String ("MidiChan");
            auto midiChanParameterName = juce::String ("MidiChan");
            auto midiChanMinValue = 1;
            auto midiChanMaxValue = 16;
            auto midiChanDefaultValue = 1;
            portLayout.add (std::make_unique<juce::AudioParameterInt> (midiChanParamID, midiChanParameterName, midiChanMinValue, midiChanMaxValue, midiChanDefaultValue));

            // Add MIDI Num parameter
            auto midiNumParamID = juce::String ("MidiNum");
            auto midiNumParameterName = juce::String ("MidiNum");
            auto midiNumMinValue = 0;
            auto midiNumMaxValue = 127;
            auto midiNumDefaultValue = 48;
            portLayout.add (std::make_unique<juce::AudioParameterInt> (midiNumParamID, midiNumParameterName, midiNumMinValue, midiNumMaxValue, midiNumDefaultValue));

            // Add MsgType parameter
            auto msgTypeParamID = juce::String ("MsgType");
            auto msgTypeParameterName = juce::String ("MsgType");
            auto msgTypeMinValue = 0;
            auto msgTypeMaxValue = birdhouse::MsgType::NumMsgTypes - 1;
            ;
            auto msgTypeDefaultValue = 0;
            auto msgTypeParam = std::make_unique<juce::AudioParameterInt> (msgTypeParamID, msgTypeParameterName, msgTypeMinValue, msgTypeMaxValue, msgTypeDefaultValue);
            portLayout.add (std::move (msgTypeParam));

            // Add Muted parameter
            auto mutedParamID = juce::String ("Muted");
            auto mutedParameterName = juce::String ("Muted");
            auto mutedDefaultValue = false;
            portLayout.add (std::make_unique<juce::AudioParameterBool> (mutedParamID, mutedParameterName, mutedDefaultValue));

            return portLayout;
        }
    };
}
