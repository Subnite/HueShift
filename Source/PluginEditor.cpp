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
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setSize (500, 500);
    setResizable(true, true);
    addAndMakeVisible(camera);
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

    camera.setBounds(bounds);
}
