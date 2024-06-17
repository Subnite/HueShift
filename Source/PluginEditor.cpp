/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HueShiftEditor::HueShiftEditor(HueShiftProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    cameraSelector(camera),
    cameraGrid(camera, p, 50),
    network(audioProcessor.hardwareListener)
{
    setSize (1500, 500);
    setResizable(true, true);

    float scale = 1080/1920.f;
    int x = 5;

    cameraGrid.SetGridSettings(13, x, int(x*scale));

    addAndMakeVisible(camera);
    addAndMakeVisible(cameraSelector);
    addAndMakeVisible(cameraGrid);

    addAndMakeVisible(network);
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
    auto upperTabsBounds = bounds.removeFromTop(bounds.getHeight()*0.05f);
    auto camSelectorBounds = upperTabsBounds.removeFromRight(upperTabsBounds.getWidth() * 0.2f);
    auto portNumberBounds = upperTabsBounds.removeFromLeft(upperTabsBounds.getWidth()*0.1f);

    cameraSelector.setBounds(camSelectorBounds);
    network.setBounds(portNumberBounds);
    camera.setBounds(bounds.removeFromRight(bounds.getWidth()*0.5f));
    cameraGrid.setBounds(bounds);

}
