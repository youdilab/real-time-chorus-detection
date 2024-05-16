/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Config.h"
#include "State.h"

//==============================================================================
/**
*/
class ChorusFindAudioProcessorEditor  : public juce::AudioProcessorEditor
    ,public juce::Button::Listener
    ,public juce::Timer
{
public:
    ChorusFindAudioProcessorEditor (ChorusFindAudioProcessor&, juce::AudioProcessorValueTreeState&, State&);
    ~ChorusFindAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ChorusFindAudioProcessor& audioProcessor;

    juce::AudioProcessorValueTreeState& apvts;
    State& procState;

    //GUI components
    juce::Slider sldChorusAmount;

    juce::TextButton btnEval;

    juce::Label lblStatus;
    juce::Label lblSoloText;
    juce::Label lblChorusText;

    //Parameter-Component attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attchChorusAmount;

    void buttonClicked(juce::Button*) override;
    void updateEnableEval();
    void updateStatusText();

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChorusFindAudioProcessorEditor)
};
