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