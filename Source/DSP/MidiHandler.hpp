#pragma once
#include "juce_core/juce_core.h"
#include <JuceHeader.h>

class MidiHandler : juce::Timer {
private:
    double startTime;
    MidiBuffer& buffer;
    unsigned int currentState = 0;
    void timerCallback() override {
        if (currentState) {
            // buffer.clear();
            
            auto message = juce::MidiMessage::noteOn(1, 60, uint8(127));
            message.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001 - startTime);

            buffer.addEvent(
                message,
                0
            );

            currentState = 0;
        }
        else {
            buffer.addEvent(
                juce::MidiMessage::noteOff(1, 60, uint8(127)),
                0
            );
            currentState = 1;
        }
    }
public:
    MidiHandler(juce::MidiBuffer& buffer, unsigned int refreshRateHz)
    : buffer(buffer) {
        startTimerHz(refreshRateHz);
        startTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
    }
};
