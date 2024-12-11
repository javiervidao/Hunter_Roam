#include "hunter_roam.h"

namespace esphome {
namespace hunter_roam {

void HunterRoam::set_pin(int pin) {
	_pin = pin;
	pinMode(pin, OUTPUT);
}

String HunterRoam::errorHint(byte error) {
	switch (error) {
		case 0:
			return String("No error.");
		case 1:
			return String("Invalid zone number.");
		case 2:
			return String("Invalid watering time.");
		case 3:
			return String("Invalid program number.");
		default:
			return String("Unknown error.");
	}
}

void HunterRoam::sendLow() {
	digitalWrite(_pin, HIGH);
	delayMicroseconds(SHORT_INTERVAL);
	digitalWrite(_pin, LOW);
	delayMicroseconds(LONG_INTERVAL);
}

void HunterRoam::sendHigh() {
	digitalWrite(_pin, HIGH);
	delayMicroseconds(LONG_INTERVAL);
	digitalWrite(_pin, LOW);
	delayMicroseconds(SHORT_INTERVAL);
}

void HunterRoam::writeBus(std::vector<byte> buffer, bool extrabit) {
	// Reset impulse
	digitalWrite(_pin, HIGH);
	delay(325); // milliseconds
	digitalWrite(_pin, LOW);
	delay(65); // milliseconds

	// Start impulse
	digitalWrite(_pin, HIGH);
	delayMicroseconds(START_INTERVAL);
	digitalWrite(_pin, LOW);
	delayMicroseconds(SHORT_INTERVAL);

	// Write the bits out
	for (auto &sendByte : buffer) {
		for (byte inner = 0; inner < 8; inner++) {
			// Send high order bits first
			(sendByte & 0x80) ? sendHigh() : sendLow();
			sendByte <<= 1;
		}
	}

	// Include an extra 1 bit
	if (extrabit) {
		sendHigh();
	}

	// Write the stop pulse
	sendLow();
}

/**	
 * Set a value with an arbitrary bit width to a bit position within a blob.
 * 
 * @param bits blob to write the value to
 * @param pos position within the blob
 * @param val to write
 * @param len in bits of the value
 */
void HunterRoam::hunterBitfield(std::vector <byte> &bits, byte pos, byte val, byte len) {
	while (len > 0) {
		if (val & 0x1) {
			bits[pos / 8] = bits[pos / 8] | 0x80 >> (pos % 8);
		} else {
			bits[pos / 8] = bits[pos / 8] & ~(0x80 >> (pos % 8));
		}
		len--;
		val = val >> 1;
		pos++;
	}
}

/**
 * Start a zone
 * 
 * @param zone zone number (1-48)
 * @param time time in minutes (0-240)
 */
byte HunterRoam::startZone(byte zone, byte time) {
	// Start out with a base frame
	std::vector<byte> buffer = {0xff,0x00,0x00,0x00,0x10,0x00,0x00,0x04,0x00,0x00,0x01,0x00,0x01,0xb8,0x3f};

	if (zone < 1 || zone > 48) {
		return 1;  // Invalid zone number
	}

	if (time < 0 || time > 240) {
		return 2;  // Invalid watering time
	}

	// The bus protocol is a little bizarre, not sure why

	// Bits 9:10 are 0x1 for zones > 12 and 0x2 otherwise
	if (zone > 12) {
		hunterBitfield(buffer, 9, 0x1, 2);
	} else {
		hunterBitfield(buffer, 9, 0x2, 2);
	}

	// Encoding zone and time into the buffer
	hunterBitfield(buffer, 23, zone + 0x17, 7);
	hunterBitfield(buffer, 36, zone + 0x17, 7);
	hunterBitfield(buffer, 49, zone + 0x23, 7);
	hunterBitfield(buffer, 62, zone + 0x23, 7);
	hunterBitfield(buffer, 75, zone + 0x2f, 7);
	hunterBitfield(buffer, 88, zone + 0x2f, 7);
	hunterBitfield(buffer, 31, time, 4);
	hunterBitfield(buffer, 44, time >> 4, 4);
	hunterBitfield(buffer, 57, time, 4);
	hunterBitfield(buffer, 70, time >> 4, 4);
	hunterBitfield(buffer, 83, time, 4);
	hunterBitfield(buffer, 96, time >> 4, 4);
	hunterBitfield(buffer, 109, zone - 1, 4);

	// Write the bits out of the bus
	writeBus(buffer, true);

	return 0;  // Success
}

/**
 * Stop a zone
 * 
 * @param zone - zone number (1-48)
 */
byte HunterRoam::stopZone(byte zone) {
	return startZone(zone, 0);  // Call startZone with 0 time to stop the zone
}

/**
 * Run a program
 * 
 * @param num - program number (1-4)
 */
byte HunterRoam::startProgram(byte num) {
	// Start with a basic program frame
	std::vector<byte> buffer = {0xff, 0x40, 0x03, 0x96, 0x09 ,0xbd ,0x7f};

	if (num < 1 || num > 4) {
		return 3;  // Invalid program number
	}

	// Program number - 1 is at bits 31:32
	hunterBitfield(buffer, 31, num - 1, 2);
	writeBus(buffer, false);

	return 0;  // Success
}

}}