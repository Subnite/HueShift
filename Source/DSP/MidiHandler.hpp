#pragma once
#include "juce_core/juce_core.h"
#include <JuceHeader.h>
#include "../Commons/ColorUtils.hpp"

namespace HueShift{

// keep data -1 if you want no change.
struct ReadDataOutput {
    int
    freezeGridIndex = -1, // counts from top left to bottom right.
    unfreezeGridIndex = -1,
    cameraHz = -1,
    toggleOctaveIndex = -1;

    static ReadDataOutput ReadData(const MidiBuffer& buffer) {
        ReadDataOutput output{};
        
        for (const auto metadata : buffer)
        {
            auto message = metadata.getMessage();
            output.freezeGridIndex = message.isNoteOn() ? NoteToGridIndex(message.getNoteNumber()) : -1;
            output.unfreezeGridIndex = message.isNoteOff() ? NoteToGridIndex(message.getNoteNumber()) : -1;

            // TODO: add functionality for finding the cameraHz and if the octave should be cycled.
        }

        return output;
    }
};

class MidiVoice {
public:
    // sends a midi message if the frequency wishes it.
    void Process(double frequency, unsigned int noteNumber, unsigned int noteLengthSamples,
        size_t sampleRate, size_t bufferSize, double startTimeSeconds, MidiBuffer& outputBuffer) const
    {
        double timeNow = Time::getMillisecondCounterHiRes() * 0.001 - startTimeSeconds;
        int samplesNow = static_cast<int>(timeNow * sampleRate);
        int samplesPerCycle = sampleRate / frequency;

        // note on messages
        int messageSamplePos = samplesNow % samplesPerCycle;
        for (messageSamplePos; messageSamplePos < bufferSize; messageSamplePos += samplesPerCycle) {
            auto message = juce::MidiMessage::noteOn(1, noteNumber, uint8(127));
            message.setTimeStamp(timeNow + messageSamplePos / sampleRate);

            outputBuffer.addEvent(message, messageSamplePos);
        }
        
        // note off messages
        int messageOffPos = (samplesNow + noteLengthSamples) % samplesPerCycle;
        for (messageOffPos; messageOffPos < bufferSize; messageOffPos += samplesPerCycle) {
            auto message = juce::MidiMessage::noteOn(1, noteNumber, uint8(127));
            message.setTimeStamp(timeNow + messageOffPos / sampleRate);

            outputBuffer.addEvent(message, messageOffPos);
        }
    }
};

// Base note = C1 (24)
class MidiHandler {
private:
    double startTime; // from when the buffer should start as a pivot point
    size_t sampleRate;
    MidiBuffer& outputBuffer;

    ReadDataOutput ReadData(const juce::MidiBuffer& buffer) {
        return ReadDataOutput::ReadData(buffer);
    }

    // do stuff to the data like freezing etc
    void ApplyData(const ReadDataOutput& data) const{

    }

public:
    MidiHandler(juce::MidiBuffer& outputBuffer)
    : outputBuffer(outputBuffer) {
        startTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
    }

    void Reset(size_t sampleRate, double startTimeSeconds) {
        this->sampleRate = sampleRate;
        this->startTime = startTimeSeconds;
    }

    void Process(const juce::MidiBuffer& inputBuffer, const std::vector<juce::Colour>& gridColours, unsigned int bufferSize) {
        // [1] read the data
        const auto inputData = ReadData(inputBuffer);
        ApplyData(inputData);

        // [2] process voices
        const MidiVoice voice{};
        for (int i = 0; i < gridColours.size() && i < 1; i++) {
            voice.Process(
                ColorInfo::GetClosestColor(gridColours[i]).frequency, // freq
                C1 + i, // note
                100, // note length samples
                sampleRate,
                bufferSize,
                startTime, // start time in seconds
                outputBuffer // buffer to write messages to
            );
        }

    };
};

}