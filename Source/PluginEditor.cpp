/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ChorusFindAudioProcessorEditor::ChorusFindAudioProcessorEditor (ChorusFindAudioProcessor& p, juce::AudioProcessorValueTreeState& valueTree, State& state)
    : AudioProcessorEditor (&p), audioProcessor (p), apvts(valueTree)
    , procState(state)
    , sldChorusAmount(juce::Slider::SliderStyle::LinearHorizontal, juce::Slider::TextEntryBoxPosition::NoTextBox)
    , btnEval(text::textEval)
    , lblSoloText(text::textSolo, text::textSolo)
    , lblChorusText(text::textChorus, text::textChorus)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 200);

    //Slider Components
    addAndMakeVisible(sldChorusAmount);
    sldChorusAmount.setEnabled(false);

    //Evaluate Button
    btnEval.addListener(this);
    addAndMakeVisible(btnEval);

    //Status Label
    lblStatus.setText("Ready...",juce::NotificationType::dontSendNotification);
    addAndMakeVisible(lblStatus);

    addAndMakeVisible(lblSoloText);
    addAndMakeVisible(lblChorusText);

    addAndMakeVisible(lblSoloPct);
    addAndMakeVisible(lblChorusPct);

    updatePcts(sldChorusAmount.getValue());

    attchChorusAmount.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, parameters::chorusAmount.id, sldChorusAmount));

    startTimer(100);
}

ChorusFindAudioProcessorEditor::~ChorusFindAudioProcessorEditor()
{
}

//==============================================================================
void ChorusFindAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void ChorusFindAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    btnEval.setBounds(20, 30, 150, 30);

    lblStatus.setBounds(20,80,150,30);

    sldChorusAmount.setBounds(20, 120, 300, 30);
    
    lblSoloText.setBounds(20, 140, 100, 30);
    lblSoloPct.setBounds(20,160,100,30);
    
    lblChorusText.setBounds(280, 140, 100, 30);
    lblChorusPct.setBounds(280, 160, 100, 30);
}

void ChorusFindAudioProcessorEditor::buttonClicked(juce::Button*)
{    
    procState.goToNextState();
    updateEnableEval();
}

void ChorusFindAudioProcessorEditor::updateEnableEval()
{
    if (procState.getPluginState() == PluginState::READY)
    {
        btnEval.setEnabled(true);
    }
    else
    {
        btnEval.setEnabled(false);
    }
}

void ChorusFindAudioProcessorEditor::updateStatusText()
{
    juce::String statusText;

    switch (procState.getPluginState())
    {
    case PluginState::READY:
        statusText = "Ready...";
        break;
    case PluginState::LISTENING:
        statusText = "Listening to audio...";
        break;
    case PluginState::CALCULATING:
        statusText = "Calculating...";
        break;
    case PluginState::BACKGROUND:
        statusText = "Calculating.....";
        break;
    default:
        break;
    }

    lblStatus.setText(statusText,juce::NotificationType::dontSendNotification);
}

void ChorusFindAudioProcessorEditor::updatePcts(float chorusPct)
{
    float soloPct = 100 - chorusPct;
    juce::String soloPctText = juce::String(soloPct) + "%";
    juce::String chorusPctText = juce::String(chorusPct) + "%";

    lblSoloPct.setText(soloPctText,juce::NotificationType::dontSendNotification);
    lblChorusPct.setText(chorusPctText, juce::NotificationType::dontSendNotification);
}

void ChorusFindAudioProcessorEditor::timerCallback()
{
    updateEnableEval();
    updateStatusText();
    updatePcts(sldChorusAmount.getValue());
}
