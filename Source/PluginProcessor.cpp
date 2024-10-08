/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HueShiftProcessor::HueShiftProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                    handler(midiOutputBuffer),
                    hardwareListener(handler, midiUpdateGuard),
                    discoveryHandler()
#endif
{   
}

HueShiftProcessor::~HueShiftProcessor()
{}

//==============================================================================
void HueShiftProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);

    std::unique_lock<std::mutex> lock(midiUpdateGuard, std::try_to_lock);
    while (!lock.owns_lock()){
        lock = std::unique_lock<std::mutex>{midiUpdateGuard, std::try_to_lock};
    }

    handler.Reset(sampleRate, Time::getMillisecondCounterHiRes() * 0.001);
}

void HueShiftProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(buffer);
    if (hadEditor && !isEditorActive) {
        hadEditor = false;
        handler.Reset(handler.GetSampleRate(), Time::getMillisecondCounterHiRes() * 0.001);
    }

    try {
        const std::lock_guard<std::mutex> lock(colourDataGuard);
        handler.Process(midiMessages, colourData, buffer.getNumSamples());
    } catch (std::logic_error&) {
        std::cout << "couldn't gain lock for midihandler from processBlock\n";
    }

    // swap with input buffer
    midiMessages = midiOutputBuffer;
    midiOutputBuffer.clear();
}

bool HueShiftProcessor::isVoiceEnabled(size_t row, size_t column, size_t amtColumns) const {
    return handler.isVoiceEnabled(column, row, amtColumns);
}

const juce::String HueShiftProcessor::getName() const
{
    return juce::String("HueShift");
}

bool HueShiftProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool HueShiftProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool HueShiftProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double HueShiftProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HueShiftProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int HueShiftProcessor::getCurrentProgram()
{
    return 0;
}

void HueShiftProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String HueShiftProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void HueShiftProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================


void HueShiftProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HueShiftProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

//==============================================================================
bool HueShiftProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* HueShiftProcessor::createEditor()
{
    hadEditor = true;
    return new HueShiftEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void HueShiftProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ignoreUnused(destData);
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void HueShiftProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HueShiftProcessor();
}
