#pragma once
// ================ General
#define MINIMUM_FREQ 20.0
#define MAXIMUM_FREQ 20000.0

// ================ MIDI
#define C1 24

// ================ Network UDP Data Receiver
/*
    udp packet example:
    f-000000001;
    = f<prefix>000000001<postfix>
    this will freeze the 1st grid index if the bytes per message is 12.
*/

#define HARDWARE_PORT 0 // OS chooses for us if it is 0
#define NUMBER_PREFIX '-' // the char before an int begins
#define NUMBER_POSTFIX ';' // the terminating char right after an int
#define BYTES_PER_MESSAGE 12 // max amt of digits + 3 for the prefixes

#define DISCOVERY_RECEIVE_PORT 8179
#define DISCOVERY_RESPONSE_PORT 8180
#define DISCOVERY_RECEIVE_MESSAGE "HS_PING"
#define DISCOVERY_RESPONSE_PREFIX "HS_" // add the port (with zero at the start if needed) after this. Port should be 04848 or 11456 for example.
#define DISCOVERY_RECEIVE_BYTES 7
#define DISCOVERY_RESPONSE_BYTES 8