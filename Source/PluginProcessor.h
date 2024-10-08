/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include <mutex>
#include <JuceHeader.h>
#include "Commons/ParameterNaming.hpp"
#include "DSP/MidiHandler.hpp"
#include "Commons/HardwareListener.hpp"

//==============================================================================

class HueShiftProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    HueShiftProcessor();
    ~HueShiftProcessor() override;

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

    //==============================================================================
    bool isVoiceEnabled(size_t row, size_t column, size_t amtColumns) const;

    std::vector<juce::Colour> colourData; // should only be written to from the editor camera.
    std::mutex colourDataGuard; // locks the colourData. ALWAYS USE IT

    std::mutex midiUpdateGuard; // locks the midihandler from being accessed from other threads. ALWAYS USE IT
    HueShift::MIDIListenerUDP hardwareListener;
    HueShift::DiscoveryHandlerUDP discoveryHandler;
    bool isEditorActive = false;
private:
    juce::MidiBuffer midiOutputBuffer;
    HueShift::MidiHandler handler;
    bool hadEditor = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HueShiftProcessor)

};
