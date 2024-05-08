/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HueShiftEditor::HueShiftEditor(HueShiftProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize (500, 500);
    setResizable(true, true);

    addAndMakeVisible(camera);
    addAndMakeVisible(cameraSelector);
}

HueShiftEditor::~HueShiftEditor()
{

}

//==============================================================================
void HueShiftEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::red.withSaturation(0.2f).withBrightness(0.35f));
}

void HueShiftEditor::resized()
{
    auto bounds = getLocalBounds();
    auto camSelectorBounds = bounds.removeFromTop(bounds.getHeight()*0.05f);

    cameraSelector.setBounds(camSelectorBounds);
    camera.setBounds(bounds);
}
