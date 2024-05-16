/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
//Initialization of static variables.
juce::ThreadPool ChorusFindAudioProcessor::sharedThredPool{};
//==============================================================================
ChorusFindAudioProcessor::ChorusFindAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
    , parameters(*this, nullptr, juce::Identifier("APVTS"), createParameterLayout())
    , paramReadChorusAmount(parameters.getRawParameterValue(parameters::chorusAmount.id))
    , paramChorusAmount(parameters.getParameter(parameters::chorusAmount.id))
{
    
}

ChorusFindAudioProcessor::~ChorusFindAudioProcessor()
{
}

//==============================================================================
const juce::String ChorusFindAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ChorusFindAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ChorusFindAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ChorusFindAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ChorusFindAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ChorusFindAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ChorusFindAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ChorusFindAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ChorusFindAudioProcessor::getProgramName (int index)
{
    return {};
}

void ChorusFindAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ChorusFindAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    bufListen.clear();
}

void ChorusFindAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ChorusFindAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ChorusFindAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    if (currState.getPluginState() == PluginState::READY)
    {
        
    }
    else if (currState.getPluginState() == PluginState::LISTENING)
    {
        juce::Logger::writeToLog("Listening");
        if (listenWriteIndex + buffer.getNumSamples() > bufListen.getNumSamples())
        {
            for (int channel = 0; channel < totalNumInputChannels; ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                bufListen.copyFrom(channel, listenWriteIndex, buffer, channel, 0, bufListen.getNumSamples()-listenWriteIndex);
            }

            currState.goToNextState();
            juce::Logger::writeToLog("Listening Completed");
            listenWriteIndex = 0;
        }
        else
        {
            for (int channel = 0; channel < totalNumInputChannels; ++channel)
            {
                auto* channelData = buffer.getWritePointer(channel);
                bufListen.copyFrom(channel, listenWriteIndex, buffer, channel, 0, buffer.getNumSamples());
            }

            listenWriteIndex += buffer.getNumSamples();
        }
        
    }
    else if (currState.getPluginState() == PluginState::CALCULATING)
    {
        juce::Logger::writeToLog("Calculating");

        //Call API from another thread.

        sharedThredPool.addJob([&]()
            {
                float api_result = callChorusDetectionAPI(getSampleRate(),24);
                paramChorusAmount->setValueNotifyingHost(api_result);
                
                juce::Logger::writeToLog("Calculation Completed.");
                currState.goToNextState();
            });
        currState.goToNextState();
        juce::Logger::writeToLog("Went to next state");
    }
    else if (currState.getPluginState() == PluginState::BACKGROUND)
    {

    }
    
}

float ChorusFindAudioProcessor::callChorusDetectionAPI(int sampleRate, int bitsPerSample)
{
    juce::File tempFile = juce::File::createTempFile("temp_audio.wav");    
    saveAudioBufferAsWav(bufListen, tempFile,sampleRate,bitsPerSample);

    juce::URL url(api::url);
    
    auto stream = std::make_unique<juce::FileInputStream>(tempFile);

    try
    {
        if (stream->openedOk())
        {
            url = url.withFileToUpload("audio", tempFile, "audio/wav");
            std::unique_ptr<juce::InputStream> responseStream = url.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress));

            juce::String response;

            try
            {
                response = responseStream->readEntireStreamAsString();
            }
            catch(const std::exception& e)
            {
                juce::Logger::writeToLog("Exception occurred: " + juce::String(e.what()));
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                    "Error",
                    "API is not accessible. Please make sure you are connected to the internet.");
                return 0.5f;
            }
            

            // Parse the JSON response
            juce::var result = juce::JSON::parse(response);

            if (result.isObject())
            {
                juce::var resultValue = result.getProperty("result", 0.5f);

                if (resultValue.isDouble())
                {
                    // Extract the floating-point value
                    double floatValue = resultValue;
                    DBG("Float value from server response: " << floatValue);
                    return floatValue;
                }
            }

            DBG("Response from server: " << response);
        }
    }
    catch (const std::exception& e)
    {
        // Handle any exceptions that occur
        juce::Logger::writeToLog("Exception occurred: " +juce::String( e.what()));
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Error",
            "API is not accessible. Please make sure you are connected to the internet.");
    }
}

void ChorusFindAudioProcessor::saveAudioBufferAsWav(const juce::AudioBuffer<float>& buffer, juce::File& fileToSave, int sampleRate, int bitsPerSample=24)
{
    std::unique_ptr<juce::AudioFormatWriter> writer(juce::WavAudioFormat().createWriterFor(
        new juce::FileOutputStream(fileToSave),
        sampleRate,
        buffer.getNumChannels(),
        bitsPerSample,
        {},
        0
    ));

    if (writer != nullptr)
    {
        writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());

        // Close the writer to finalize the file
        delete writer.release();
    }
    else
    {
        juce::Logger::writeToLog("Error creating .wav writer.");

        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Error",
            "Error saving .wav file.");
    }
}

//==============================================================================
bool ChorusFindAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ChorusFindAudioProcessor::createEditor()
{
    return new ChorusFindAudioProcessorEditor (*this, parameters,currState);
}

//==============================================================================
void ChorusFindAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ChorusFindAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChorusFindAudioProcessor();
}
