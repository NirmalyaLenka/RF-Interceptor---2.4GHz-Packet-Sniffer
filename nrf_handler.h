/*
  nrf_handler.h - NRF24L01 Module Interface
  ==========================================
  This header file declares the functions and data structures used
  to communicate with both NRF24 radio modules.

  You do not need to edit this file. The settings are in config.h.
*/

#pragma once
#include <Arduino.h>
#include <RF24.h>
#include "config.h"

// ---------------------------------------------------------------------------
// PacketResult - Holds everything about one captured radio packet
// ---------------------------------------------------------------------------
struct PacketResult {
  bool    packetFound;            // Was a packet actually found?
  uint8_t channel;                // Which channel was it found on? (0-124)
  int8_t  rssi;                   // Signal strength (NRF24 does not give exact RSSI,
                                  // we use carrier detect as a proxy: -64 or 0)
  uint8_t data[MAX_PAYLOAD_SIZE]; // Raw bytes of the packet
  uint8_t length;                 // How many bytes are in the packet
  uint32_t timestamp;             // millis() when the packet was captured
};

// ---------------------------------------------------------------------------
// Scanner functions (NRF Module 1)
// ---------------------------------------------------------------------------

// Call this in setup(). Returns true if the module was found, false if not.
bool nrfScanner_init();

// Scan the next channel in sequence. Returns a PacketResult.
// Call this every loop iteration.
PacketResult nrfScanner_scanNextChannel();

// Returns the channel number the scanner is currently listening on.
uint8_t nrfScanner_getCurrentChannel();

// Prints a table showing how many packets were found on each channel.
void nrfScanner_printStatus();

// ---------------------------------------------------------------------------
// Transmitter functions (NRF Module 2)
// ---------------------------------------------------------------------------

// Call this in setup(). Returns true if the module was found, false if not.
bool nrfTransmitter_init();

// Store the last captured packet so it can be replayed later.
void nrfTransmitter_storeLastPacket(PacketResult &pkt);

// Transmit the stored packet on the same channel it was captured from.
void nrfTransmitter_replayLastPacket();

// Send an arbitrary packet on a specific channel.
bool nrfTransmitter_sendPacket(uint8_t channel, uint8_t *data, uint8_t length);
