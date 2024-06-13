#pragma once

#include "JuceHeader.h"
#include <string>
#include "../DSP/MidiHandler.hpp"
#include "ParameterNaming.hpp"

namespace HueShift {

class HardwareListener :
	public juce::Thread
{
private:
	MidiHandler& midiHandler;
	std::mutex& midiGuard;
	const int updateHz;

	juce::DatagramSocket udpSocket;
	char readBuffer[BYTES_PER_MESSAGE];
	inline static int port = -1; // it doesn't connect to this port, but updates it to whichever port it connects to.

	// will keep trying to bind socket
	void SetupUDPWithBlocking() {
		bool wasBound = false;
		int readStatus = 0;

		do {
			wasBound = udpSocket.bindToPort(HARDWARE_PORT);
		} while (!wasBound);

		port = udpSocket.getBoundPort();
		std::cout << "bound to port: " << port << "\n";
		
		do {
			readStatus = udpSocket.waitUntilReady(true, 1000);
		} while (readStatus != 1);

	}

	// returns -69 if not found
	int ReadNumberFromData(const std::string& data) const {
		const char intPrefix = NUMBER_PREFIX;
		const char intPostfix = NUMBER_POSTFIX;

		auto foundPrefix = data.find(intPrefix);
		auto foundPostfix = data.find(intPostfix);
		if (foundPrefix == std::string::npos
			|| foundPostfix == std::string::npos
			|| foundPostfix < foundPostfix)
		{
			return nonum;
		}

		std::string numberS = data.substr(foundPrefix+1, foundPostfix - foundPrefix); // + 1 because it is a char, not word
		return std::stoi(numberS); // stoi deals with 0s in the beginning.
	}

	void ApplyDataMidi(const std::string& data) {
		ReadDataOutput outputData{};
		
		if (data[0] == 'f'){ // toggle freeze
			auto number = ReadNumberFromData(data);
			if (number != nonum) outputData.freezeGridIndexes.push_back(number);
		}
		else if (data[0] == 'o'){ // toggle freeze
			auto number = ReadNumberFromData(data);
			if (number != nonum) outputData.toggleOctaveIndexes.push_back(number);
		}
		else if (data[0] == 'c'){ // toggle freeze
			auto number = ReadNumberFromData(data);
			if (number != nonum) outputData.cameraHz.push_back(number);
		}
		else {
			return; // nothing to change
		}


		bool wasApplied = false;

		while(!wasApplied){
			try {
				const std::lock_guard<std::mutex> lock(midiGuard);
				midiHandler.ApplyData(outputData);
				wasApplied = true;
			} catch (std::logic_error&) {
				std::cout << "couldn't gain lock\n";
			}
		}
	}
public:
	inline const static int nonum = -498416819; // returned by ReadNumberFromData when the format is incorrect.

	HardwareListener(MidiHandler& midiHandler, std::mutex& midiUpdateLock, const int intervalHz = 20) :
		juce::Thread("Hardware MIDI Listener"),
		midiHandler(midiHandler),
		midiGuard(midiUpdateLock),
		updateHz(intervalHz),
		udpSocket(false) // false because it is read-only
	{
		startThread(juce::Thread::Priority::high);
	}

	~HardwareListener() override {
		// stop connection
		stopThread(2000); // give 2000 ms to stop
		udpSocket.shutdown();
	}

	void run() override {
		// make connection
		SetupUDPWithBlocking();

		while (!threadShouldExit()){
			wait(1000/updateHz); // runs intervalHz per second minimum

			int bytesRead = udpSocket.read(readBuffer, sizeof(readBuffer), true); // sizeof buffer should be BYTES_PER_MESSAGE
			if (bytesRead == -1) {
				std::cout << "error reading bytes\n";
				continue;
			}

			auto message = std::string(readBuffer);
			ApplyDataMidi(message);
		}
	}

	// returns -1 when not connected.
	int GetActivePort() const {
		return port;
	}
};

}
