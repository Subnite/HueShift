#pragma once
#include <JuceHeader.h>

class MidiHandler : juce::Timer {
private:
    MidiBuffer& buffer;
    unsigned int currentState = 0;
    void timerCallback() override {
        if (currentState) {
            // buffer.clear();
            buffer.addEvent(
                juce::MidiMessage::noteOn(1, 66, uint8(127)),
                0
            );
            currentState = 0;
        }
        else {
            buffer.addEvent(
                juce::MidiMessage::noteOff(1, 66, uint8(127)),
                0
            );
            currentState = 1;
        }
    }
public:
    MidiHandler(juce::MidiBuffer& buffer, unsigned int refreshRateHz)
    : buffer(buffer) {
        startTimerHz(refreshRateHz);
    }
};