#pragma once

#include "BinaryData.h"
#include "PluginProcessor.h"
#include "gui/BirdHouseLookAndFeel.h"
#include "gui/TextEditorAttachment.h"
#include "melatonin_inspector/melatonin_inspector.h"

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

    // Port
    juce::TextEditor portEditor;
    // Parameter attachment
    std::unique_ptr<birdhouse::TextEditorAttachment<int>> portAttachment {
        std::make_unique<birdhouse::TextEditorAttachment<int>> (processorRef.parameters, portEditor, nullptr)
    };


    //melatonin::Inspector inspector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
