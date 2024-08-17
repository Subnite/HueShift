#pragma once
#include "juce_core/juce_core.h"
#include <JuceHeader.h>
#include "../Commons/ColorUtils.hpp"

namespace HueShift{

// keep data -1 if you want no change.
struct ReadDataOutput {
    std::vector<int> freezeGridIndexes{}; // counts from top left to bottom right.
    std::vector<int> cameraHz{}; // uses last index to apply Hz
    std::vector<int> toggleOctaveIndexes{};
    std::vector<int> selectGridIndex{};

    static ReadDataOutput ReadData(const MidiBuffer& buffer) {
        ReadDataOutput output{};
        
        for (const auto metadata : buffer)
        {
            auto message = metadata.getMessage();

            // when note is not but not velocity zero, add it to freeze grid indexes
            if (message.isNoteOn(false)) output.freezeGridIndexes.push_back(NoteToGridIndex(message.getNoteNumber()));
            
            // when the note is on and the velocity is zero, add it to octave toggle indexes
            else if (!message.isNoteOn(false) && message.isNoteOn(true)) output.toggleOctaveIndexes.push_back(NoteToGridIndex(message.getNoteNumber()));

            // TODO: add functionality for finding the cameraHz
        }

        return output;
    }
};

class MidiVoice {
private:
    mutable double prevFrequency;
    bool isFrozen = false;
    bool isEnabled = false;
    size_t currentOctaveCycleIndex = 0;
    std::vector<float> octaveMultipliers = {0.5f, 1.f, 0.25f};

public:
    // sends a midi message if the frequency wishes it.
    void Process(double frequency, unsigned int noteNumber, unsigned int noteLengthSamples,
        size_t sampleRate, size_t bufferSize, const unsigned int& startTimeSamples, const unsigned int& timeNowSamples, MidiBuffer& outputBuffer) const
    {
        if (!isEnabled) return;
        if (!isFrozen) prevFrequency = frequency;

        auto timeNow = Time::getMillisecondCounterHiRes() * 0.001;
        auto samplesNow = timeNowSamples - startTimeSamples;
        unsigned int samplesPerCycle = static_cast<unsigned int>(sampleRate / (prevFrequency * octaveMultipliers[currentOctaveCycleIndex]));

        // note on messages
        unsigned int messageSamplePos = samplesNow % samplesPerCycle;
        for (messageSamplePos; messageSamplePos < bufferSize; messageSamplePos += samplesPerCycle) {
            auto message = juce::MidiMessage::noteOn(1, noteNumber, uint8(127));
            message.setTimeStamp(timeNow + (messageSamplePos*1.f) / sampleRate);

            outputBuffer.addEvent(message, messageSamplePos);
        }
        
        // note off messages
        unsigned int messageOffPos = (samplesNow + noteLengthSamples) % samplesPerCycle;
        for (messageOffPos; messageOffPos < bufferSize; messageOffPos += samplesPerCycle) {
            auto message = juce::MidiMessage::noteOff(1, noteNumber, uint8(127));
            message.setTimeStamp(timeNow + (messageOffPos*1.f) / sampleRate);

            outputBuffer.addEvent(message, messageOffPos);
        }
    }

    void ToggleFreeze() {
        isFrozen = !isFrozen;
    }

    void ToggleSelect() {
        isEnabled = !isEnabled;
    }

    void ToggleOctave() {
        // cycle up in the octave vector if you aren't on the last one.
        currentOctaveCycleIndex = currentOctaveCycleIndex < octaveMultipliers.size()-1 ? currentOctaveCycleIndex+1 : 0;
    }

    void SetFrequencyMultipliers(std::vector<float> newFrequencyMultipliers) {
        currentOctaveCycleIndex = 0;
        octaveMultipliers = newFrequencyMultipliers;
    }

    bool isVoiceEnabled() const {
        return isEnabled;
    }
};

// Base note = C1 (24)
class MidiHandler {
private:
    unsigned int startTimeSamples; // from when the buffer should start as a pivot point
    unsigned int timeElapsedSamples = 0;
    size_t sampleRate = 48000;
    MidiBuffer& outputBuffer;
    std::vector<MidiVoice> voices{};
    bool readyToRead = false;

    ReadDataOutput ReadData(const juce::MidiBuffer& buffer) const {
        return ReadDataOutput::ReadData(buffer);
    }

    void ProcessVoices(const std::vector<juce::Colour>& gridColours, unsigned int bufferSize) {
        // firstly make sure the size of the voices vector is the same as gridColours without removing all entries.
        const int sizeDiff = gridColours.size() - voices.size();
        if (sizeDiff != 0){
            readyToRead = false;
        }
        if (sizeDiff > 0) {
            for (auto i = 0; i < sizeDiff; i++) {
                voices.push_back(MidiVoice{});
            }
            readyToRead = true;
        } else if (sizeDiff < 0) {
            for (auto i = 0; i < abs(sizeDiff); i++){
                voices.pop_back();
            }
            readyToRead = true;
        }

        // process all voices
        for (int i = 0; i < gridColours.size() && i < voices.size()/* && i < 2*/; i++) {
            auto& voice = voices[i];
            voice.Process(
                ColorInfo::GetClosestColor(gridColours[i]).frequency, // freq
                C1 + i, // note
                4000, // note length samples
                sampleRate,
                bufferSize,
                startTimeSamples, // start time in seconds
                startTimeSamples + timeElapsedSamples,
                outputBuffer // buffer to write messages to
            );
        }
    }

public:
    MidiHandler(juce::MidiBuffer& outputBuffer)
    : outputBuffer(outputBuffer) {
        startTimeSamples = static_cast<unsigned int>(juce::Time::getMillisecondCounterHiRes() * 0.001 * sampleRate);
    }

    void Reset(size_t sampleRate, double startTimeSamples) {
        this->sampleRate = sampleRate;
        this->startTimeSamples = startTimeSamples;
    }

    void Process(const juce::MidiBuffer& inputBuffer, const std::vector<juce::Colour>& gridColours, unsigned int bufferSize) {
        // [1] read the data
        const auto inputData = ReadData(inputBuffer);
        ApplyData(inputData);

        // [2] process voices
        ProcessVoices(gridColours, bufferSize);

        timeElapsedSamples += bufferSize;
    };

    // do stuff to the data like freezing etc
    void ApplyData(const ReadDataOutput& data) {
        for (const auto& freezeIdx : data.freezeGridIndexes) {
            if (freezeIdx < voices.size()) voices[freezeIdx].ToggleFreeze();
        }

        for (const auto& octaveIndex : data.toggleOctaveIndexes) {
            if (octaveIndex < voices.size()) voices[octaveIndex].ToggleOctave();
        }

        for (const auto& selectIndex : data.selectGridIndex) {
            if (selectIndex < voices.size()) voices[selectIndex].ToggleSelect();
        }
    }

    // row and column are 0 based
    bool isVoiceEnabled(size_t column, size_t row, size_t amtColumns) const {
        size_t index = amtColumns * row + column;
        // return false;
        try{
            if (!readyToRead) return false;
            if (index < voices.size()){
                // jassert(index < voices.size());
                const auto& voice = voices[index];
                auto res = voice.isVoiceEnabled();
                return res;
            }

        } catch (std::logic_error&){
            std::cerr << "couldn't find voice\n";
            return false;
        }
    }
};

}