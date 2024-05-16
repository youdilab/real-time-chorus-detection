/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Config.h"
#include "State.h"

//==============================================================================
/**
*/
class ChorusFindAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ChorusFindAudioProcessor();
    ~ChorusFindAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;    

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //Value Tree State.
    juce::AudioProcessorValueTreeState parameters;

    //Chorus Calculation State
    State currState;

    //Listening Buffer
    juce::AudioBuffer<float> bufListen{2, 48000 * 2};
    int listenWriteIndex = 0;

    static juce::ThreadPool sharedThredPool;

    float callChorusDetectionAPI(int sampleRate, int bitsPerSample = 24);
    void saveAudioBufferAsWav(const juce::AudioBuffer<float>& buffer, juce::File& fileToSave, int sampleRate, int bitsPerSample);

    //Adding parameters to parameter layout.
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout params;

        //Lambda function to add juce::AudioParameterFloat
        const auto addFloatParameter = [&](parameters::audioParameterFloat paramFloat)
        {
            params.add(std::make_unique<juce::AudioParameterFloat>(paramFloat.id, paramFloat.name, paramFloat.minValue, paramFloat.maxValue, paramFloat.defaultValue));
        };

        //Lambda function to add juce::AudioParameterInt
        const auto addIntParameter = [&](parameters::audioParameterInt paramInt)
        {
            params.add(std::make_unique<juce::AudioParameterFloat>(paramInt.id, paramInt.name, paramInt.minValue, paramInt.maxValue, paramInt.defaultValue));
        };

        //Adding parameters using above lambda functions.
        addFloatParameter(parameters::volume1);
        addFloatParameter(parameters::volume2);
        addFloatParameter(parameters::chorusAmount);
        addIntParameter(parameters::chorusState);

        return params;
    };

    //Accessing parameters.

    //Reading parameter values.
    std::atomic<float>* paramReadVolume1 = nullptr;
    std::atomic<float>* paramReadVolume2 = nullptr;

    //Modifying parameter values.
    juce::RangedAudioParameter* paramVolume1 = nullptr;
    juce::RangedAudioParameter* paramVolume2 = nullptr;
    juce::RangedAudioParameter* paramChorusAmount = nullptr;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChorusFindAudioProcessor)
};
