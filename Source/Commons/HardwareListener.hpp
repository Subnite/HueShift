#pragma once

#include "JuceHeader.h"
#include <string>
#include "../DSP/MidiHandler.hpp"
#include "ParameterNaming.hpp"

namespace HueShift {

class MIDIListenerUDP :
	public juce::Thread
{
private:
	MidiHandler& midiHandler;
	std::mutex& midiGuard;
	const int updateHz;

	juce::DatagramSocket receiverSocket;
	char readBuffer[BYTES_PER_MESSAGE];
	inline static int port = -1; // it doesn't connect to this port, but updates it to whichever port it connects to.

	// will keep trying to bind socket
	void SetupUDPWithBlocking() {
		bool wasBound = false;
		int readStatus = 0;

		do {
			wasBound = receiverSocket.bindToPort(HARDWARE_PORT);
		} while (!wasBound && !threadShouldExit());

		port = receiverSocket.getBoundPort();
		std::cout << "bound to port: " << port << "\n";
		
		do {
			readStatus = receiverSocket.waitUntilReady(true, 333);
		} while (readStatus != 1 && !threadShouldExit());

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

		std::cout << "Read: " << data << "\n";
		
		if (data[0] == 'f'){ // toggle freeze
			auto number = ReadNumberFromData(data);
			if (number != nonum) outputData.freezeGridIndexes.push_back(number);
		}
		else if (data[0] == 'o'){ // toggle octave
			auto number = ReadNumberFromData(data);
			if (number != nonum) outputData.toggleOctaveIndexes.push_back(number);
		}
		else if (data[0] == 'c'){ // toggle octave
			auto number = ReadNumberFromData(data);
			if (number != nonum) outputData.cameraHz.push_back(number);
		}
		else if (data[0] == 's'){ // toggle select
			auto number = ReadNumberFromData(data);
			if (number != nonum) outputData.selectGridIndex.push_back(number);
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

	MIDIListenerUDP(MidiHandler& midiHandler, std::mutex& midiUpdateLock, const int intervalHz = 20) :
		juce::Thread("Hardware MIDI Listener"),
		midiHandler(midiHandler),
		midiGuard(midiUpdateLock),
		updateHz(intervalHz),
		receiverSocket(false) // false because it is read-only
	{
		startThread(juce::Thread::Priority::high);
	}

	~MIDIListenerUDP() override {
		stopThread(3000); // give 3000 ms to stop
		receiverSocket.shutdown(); // stop socket connection
	}

	void run() override {
		// make connection
		SetupUDPWithBlocking();

		while (!threadShouldExit()){
			wait(1000/updateHz); // runs intervalHz per second minimum

			int bytesRead = 0;
			do {
				if (threadShouldExit()) return;
				bytesRead = receiverSocket.read(readBuffer, sizeof(readBuffer), false); // sizeof buffer should be BYTES_PER_MESSAGE
				wait(10);

			} while (bytesRead < sizeof(readBuffer) && bytesRead != -1); // to make sure the program can exit when it is destroyed.

			if (bytesRead == -1) {
				std::cout << "error reading bytes\n";
				continue; // skip the message.
			}

			auto message = std::string(readBuffer);
			ApplyDataMidi(message);
		}
	}

	// returns -1 when not connected.
	static int GetActivePort() {
		return port;
	}
};


// ==============================================================================


class DiscoveryHandlerUDP :
	public juce::Thread
{
private:
	juce::DatagramSocket receiverSocket;
	juce::DatagramSocket responseSocket;
	bool connectedResponseSocket = false;
	const unsigned int subnetAmt = 1;

	// will keep trying to bind socket
	void SetupUDPWithBlocking() {
		bool wasBound = false;
		int readStatus = 0;

		do {
			wasBound = receiverSocket.bindToPort(DISCOVERY_RECEIVE_PORT);
		} while (!wasBound && !threadShouldExit());

		std::cout << "discovery receiver bound to port: " << DISCOVERY_RECEIVE_PORT << "\n";
		
		// do {
		// 	readStatus = receiverSocket.waitUntilReady(true, 333);
		// } while (readStatus != 1 && !threadShouldExit());

		std::cout << "discovery receiver ready to read\n";
	}

	void SendDiscoveryResponse(const std::string& targetIP) {
		// bool wasBound = false;
		// int readStatus = 0;

		// if (!connectedResponseSocket) { // wasn't connected before
		// 	do {
		// 		wasBound = responseSocket.bindToPort(0);
		// 	} while (!wasBound && !threadShouldExit() && !connectedResponseSocket); // safety connected bool
			
		// 	connectedResponseSocket = true;
		// 	std::cout << "discovery response bound to port: " << responseSocket.getBoundPort() << "\n";
		// }

		// const auto message = juce::String(DISCOVERY_RESPONSE_PREFIX) + ();

		int status = -1;
		do {
			auto port = MIDIListenerUDP::GetActivePort();
			std::string portString = (port < 10000) ? "0" + std::to_string(port) : std::to_string(port);

			portString = std::string(DISCOVERY_RESPONSE_PREFIX) + portString;
			const char* srcBuff = portString.c_str();
			
			status = responseSocket.write(targetIP, DISCOVERY_RESPONSE_PORT, srcBuff, DISCOVERY_RESPONSE_BYTES);
			wait(5);
		} while (status < DISCOVERY_RESPONSE_BYTES && status != -1);

		if (status == -1) std::cerr << "couldn't send message!\n";
		// else std::cout << "sent message!\n";
	}

public:
	DiscoveryHandlerUDP() :
		juce::Thread("Hardware Discovery Listener"),
		receiverSocket(false), // false because it is read-only
		responseSocket(true)
	{
		receiverSocket.setEnablePortReuse(true);
		responseSocket.setEnablePortReuse(true);

		startThread(juce::Thread::Priority::high);
	}

	~DiscoveryHandlerUDP() override {
		stopThread(3000); // give 3000 ms to stop
		receiverSocket.shutdown(); // stop socket connection
		responseSocket.shutdown();
	}

	void run() override {
		// make connection
		SetupUDPWithBlocking();


		while (!threadShouldExit()){
		std::cout << "listening??\n";
			char readBuffer[DISCOVERY_RECEIVE_BYTES];
			wait(10); // wait 10 ms

			int bytesRead = 0;
			do {
				if (threadShouldExit()) return;
				bytesRead = receiverSocket.read(readBuffer, sizeof(readBuffer), false); // sizeof buffer should be BYTES_PER_MESSAGE
				wait(10);

			} while (bytesRead < sizeof(readBuffer) && bytesRead != -1); // to make sure the program can exit when it is destroyed.

			if (bytesRead == -1) {
				std::cout << "error reading bytes\n";
				continue; // skip the message.
			}

			std::string bMsg; // since the gaw damn bytes are always broken
			for (auto c : readBuffer) {
				bMsg += c;
			}

			auto bufferMessage = bMsg;
			auto expectedMessage = std::string(DISCOVERY_RECEIVE_MESSAGE);
			if (bufferMessage == expectedMessage) {
				//auto localAddr = IPAddress::getLocalAddress().toString();
				auto allAddr = IPAddress::getAllAddresses();
				std::cout << "responding to " + std::to_string(allAddr.size()) + " ip's.\n";
				for (const auto& addr : allAddr) {
					juce::String changedAddr = addr.toString();

					// add 255's
					for (int i = 0; i < subnetAmt; i++) {
						changedAddr = changedAddr.upToLastOccurrenceOf(".", false, false);
					}
					for (int i = 0; i < subnetAmt; i++) {
						changedAddr.append(".255", 4);
					}
					
					SendDiscoveryResponse(changedAddr.toStdString());
				}
				// SendDiscoveryResponse("255.255.255.255");
			}
		}
	}
};

}
