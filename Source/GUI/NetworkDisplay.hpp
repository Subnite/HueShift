#pragma once

#include "JuceHeader.h"
#include "../Commons/HardwareListener.hpp"

namespace HueShift{

class NetworkDisplay : public juce::Component, public juce::Timer {
private:
    int port = 0;
    const MIDIListenerUDP& hardwareListener;
    juce::Label label;
public:
    NetworkDisplay(const MIDIListenerUDP& hardwareListener)
    :   hardwareListener(hardwareListener) {
        startTimerHz(1);
        label.setText("Port: 0000", juce::NotificationType::dontSendNotification);
        addAndMakeVisible(label);
    }

    void timerCallback() override {
        auto prevPort = port;
        port = hardwareListener.GetActivePort();
        if (port != prevPort)
            label.setText("Port: " + std::to_string(port), juce::NotificationType::dontSendNotification);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::black.withAlpha(.5f));

    }

    void resized() override {
        label.setBounds(getLocalBounds());
    }
};


}