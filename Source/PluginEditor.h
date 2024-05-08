/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/Camera.h"
#include "GUI/CameraSelector.hpp"
#include "GUI/CameraGrid.hpp"

//==============================================================================
/**
*/
class HueShiftEditor  : public juce::AudioProcessorEditor
{
public:
    HueShiftEditor (HueShiftProcessor&);
    ~HueShiftEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    HueShiftProcessor& audioProcessor;
    
    HueShift::Camera camera{};
    HueShift::CameraSelector cameraSelector;
    HueShift::CameraGrid cameraGrid; 

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HueShiftEditor)
};
