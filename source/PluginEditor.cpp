#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)//, inspector { *this }
{
    DBG ("Constructing PluginEditor");
    // Set default look and feel
    lookAndFeel = std::make_unique<BirdHouse::BirdHouseLookAndFeel>();
    juce::LookAndFeel::setDefaultLookAndFeel (lookAndFeel.get());

    auto defaultFontSize = 20.0f;

    // Labels
    auto labelFont = juce::Font (defaultFontSize, juce::Font::bold);
    auto labelColour = BirdHouse::Colours::blue;
 
    // Port
    auto portParam = processorRef.parameters.getParameter ("Port");
    auto port = static_cast<juce::AudioParameterInt*> (portParam)->get();
    portEditor.setFont (juce::Font (defaultFontSize, juce::Font::plain));
    portEditor.setJustification (juce::Justification::centred);
    portEditor.setText (juce::String (port));
    portEditor.setInputRestrictions (5, "1234567890");

    portAttachment->attach ("Port");

    addAndMakeVisible (portEditor);

    // Port label
    portLabel.setFont (labelFont);
    portLabel.setColour (juce::Label::textColourId, labelColour);
    portLabel.setText ("Port", juce::dontSendNotification);
    portLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (portLabel);

    connectionStatusLabel.setFont (labelFont);
    connectionStatusLabel.setColour (juce::Label::textColourId, BirdHouse::Colours::fg);
    connectionStatusLabel.setText (processorRef.isConnected() ? "Connected" : "Disconnected",
        juce::dontSendNotification);
    connectionStatusLabel.setJustificationType (juce::Justification::centred);

    addAndMakeVisible (connectionStatusLabel);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 200);

    // Allow resizing
    setResizable (false, false);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // Update connection status
    auto connectionStatus = processorRef.isConnected();

    if (connectionStatus)
    {
        // portEditor.applyColourToAllText (BirdHouse::Colours::green, true);
        connectionStatusLabel.setText ("Connected", juce::dontSendNotification);
        connectionStatusLabel.setColour (juce::Label::textColourId, BirdHouse::Colours::green);
    }
    else
    {
        // portEditor.applyColourToAllText (BirdHouse::Colours::red, true);

        connectionStatusLabel.setColour (juce::Label::textColourId, BirdHouse::Colours::red);
        connectionStatusLabel.setText ("Disconnected", juce::dontSendNotification);
    }
}

void PluginEditor::resized()
{
    juce::FlexBox fb;
    fb.flexWrap = juce::FlexBox::Wrap::wrap;
    fb.justifyContent = juce::FlexBox::JustifyContent::center;
    fb.alignContent = juce::FlexBox::AlignContent::center;

    fb.items.add (juce::FlexItem (portLabel).withMinWidth (50.0f).withMinHeight (50.0f));
    fb.items.add (juce::FlexItem (portEditor).withMinWidth (50.0f).withMinHeight (50.0f));
    fb.items.add (juce::FlexItem (connectionStatusLabel).withMinWidth (100.0f).withMinHeight (50.0f));

    fb.performLayout (getLocalBounds());
    /*
    auto area = getLocalBounds();
    auto itemHeight = static_cast<int> (static_cast<float> (area.getHeight()) / 2.5f);

    // Title area at the top
    // titleLabel.setBounds (area.removeFromTop (itemHeight));

    // Layout for portLabel, portEditor, and hyperlinkButton at the bottom
    auto bottomArea = area.removeFromBottom (itemHeight); // Reserve space at the bottom

    // Split the bottom area into three parts
    auto portEditorWidth = static_cast<int> (bottomArea.getWidth() * (1.0f / 12.0f));

    // Pad from left
    bottomArea.removeFromRight (static_cast<int> (getLocalBounds().getWidth() / 10.0f));
    // bottomArea.removeFromLeft (static_cast<int> (getLocalBounds().getWidth() / 8.0f));

    // Place portLabel on the left side of the bottom area
    portLabel.setBounds (bottomArea.removeFromLeft (portEditorWidth));

    // Place portEditor next to portLabel
    portEditor.setBounds (bottomArea.removeFromLeft (portEditorWidth));

    // Connection status
    connectionStatusTitleLabel.setBounds (bottomArea.removeFromLeft (portEditorWidth * 2));
    connectionStatusLabel.setBounds (bottomArea.removeFromLeft (portEditorWidth * 2));
    */
 }
