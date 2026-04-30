/*
  nrf_handler.cpp - NRF24L01 Module Implementation
  =================================================
  This file contains the actual code that runs both NRF24 modules.
  Module 1 is the scanner. It hops through all 125 channels looking
  for radio traffic. Module 2 is the transmitter. It can replay
  anything the scanner captures.
*/

#include "nrf_handler.h"

// ---------------------------------------------------------------------------
// Module objects
// RF24(CE_pin, CSN_pin) - These must match your wiring
// ---------------------------------------------------------------------------
static RF24 scannerRadio(SCANNER_CE_PIN, SCANNER_CSN_PIN);
static RF24 txRadio(TX_CE_PIN, TX_CSN_PIN);

// Internal state
static uint8_t     currentChannel = CHANNEL_MIN;
static uint32_t    channelPacketCounts[125] = {0}; // Count per channel
static PacketResult lastCapturedPacket;
static bool        hasStoredPacket = false;

// A fixed address we use for the scanner to listen on.
// NRF24 requires an address to receive packets, but in promiscuous-style
// scanning we use a simple address and catch whatever traffic happens to
// match at the byte level.
static const uint64_t SCANNER_ADDRESS = 0xABCDABCD71LL;

// ---------------------------------------------------------------------------
// nrfScanner_init()
// ---------------------------------------------------------------------------
bool nrfScanner_init() {
  // SPI.begin() uses the pins defined in config.h
  // If you are using a custom SPI bus, adjust this line.
  if (!scannerRadio.begin()) {
    return false;
  }

  // Verify the module is actually responding
  if (!scannerRadio.isChipConnected()) {
    return false;
  }

  scannerRadio.setPALevel(RF24_PA_LOW);         // Low power for scanning
  scannerRadio.setDataRate(RF24_2MBPS);         // 2Mbps to match most devices
  scannerRadio.setAutoAck(false);               // No acknowledgement needed
  scannerRadio.setPayloadSize(MAX_PAYLOAD_SIZE);
  scannerRadio.setChannel(CHANNEL_MIN);
  scannerRadio.openReadingPipe(1, SCANNER_ADDRESS);
  scannerRadio.startListening();

  return true;
}

// ---------------------------------------------------------------------------
// nrfScanner_scanNextChannel()
// ---------------------------------------------------------------------------
PacketResult nrfScanner_scanNextChannel() {
  PacketResult result;
  result.packetFound = false;
  result.channel     = currentChannel;
  result.rssi        = 0;
  result.length      = 0;
  result.timestamp   = millis();

  // Move to the next channel
  scannerRadio.setChannel(currentChannel);
  scannerRadio.startListening();

  // Wait for the dwell time to let signals arrive
  delayMicroseconds(DWELL_TIME_US);

  // Check for carrier (radio energy detected on this channel)
  bool carrierDetected = scannerRadio.testCarrier();

  // Check if a full packet was received
  if (scannerRadio.available()) {
    result.length = MAX_PAYLOAD_SIZE;
    scannerRadio.read(result.data, result.length);
    result.packetFound = true;
    result.rssi = carrierDetected ? -64 : -90; // Rough estimate
    channelPacketCounts[currentChannel]++;
  }

  // Advance to the next channel, wrapping around at the top
  currentChannel++;
  if (currentChannel > CHANNEL_MAX) {
    currentChannel = CHANNEL_MIN;
  }

  return result;
}

// ---------------------------------------------------------------------------
// nrfScanner_getCurrentChannel()
// ---------------------------------------------------------------------------
uint8_t nrfScanner_getCurrentChannel() {
  return currentChannel;
}

// ---------------------------------------------------------------------------
// nrfScanner_printStatus()
// ---------------------------------------------------------------------------
void nrfScanner_printStatus() {
  Serial.println();
  Serial.println("Channel activity map (channels with at least 1 packet):");
  Serial.println("CH   FREQ(MHz)  PACKETS");
  Serial.println("---  ---------  -------");

  for (int ch = CHANNEL_MIN; ch <= CHANNEL_MAX; ch++) {
    if (channelPacketCounts[ch] > 0) {
      Serial.printf("%3d  %4d       %lu\n",
        ch,
        2400 + ch,
        channelPacketCounts[ch]);
    }
  }
  Serial.println();
}

// ---------------------------------------------------------------------------
// nrfTransmitter_init()
// ---------------------------------------------------------------------------
bool nrfTransmitter_init() {
  if (!txRadio.begin()) {
    return false;
  }

  if (!txRadio.isChipConnected()) {
    return false;
  }

  txRadio.setPALevel(RF24_PA_HIGH);        // High power for transmitting
  txRadio.setDataRate(RF24_2MBPS);
  txRadio.setAutoAck(false);
  txRadio.setPayloadSize(MAX_PAYLOAD_SIZE);
  txRadio.stopListening();                 // Transmitter does not listen by default

  return true;
}

// ---------------------------------------------------------------------------
// nrfTransmitter_storeLastPacket()
// ---------------------------------------------------------------------------
void nrfTransmitter_storeLastPacket(PacketResult &pkt) {
  if (pkt.packetFound) {
    lastCapturedPacket = pkt;
    hasStoredPacket = true;
  }
}

// ---------------------------------------------------------------------------
// nrfTransmitter_replayLastPacket()
// ---------------------------------------------------------------------------
void nrfTransmitter_replayLastPacket() {
  if (!hasStoredPacket) {
    Serial.println("[TX] No packet stored yet. Capture something first.");
    return;
  }

  bool ok = nrfTransmitter_sendPacket(
    lastCapturedPacket.channel,
    lastCapturedPacket.data,
    lastCapturedPacket.length
  );

  if (ok) {
    Serial.print("[TX] Replayed packet on channel ");
    Serial.println(lastCapturedPacket.channel);
  } else {
    Serial.println("[TX] Replay failed. Check transmitter module wiring.");
  }
}

// ---------------------------------------------------------------------------
// nrfTransmitter_sendPacket()
// ---------------------------------------------------------------------------
bool nrfTransmitter_sendPacket(uint8_t channel, uint8_t *data, uint8_t length) {
  txRadio.stopListening();
  txRadio.setChannel(channel);
  txRadio.openWritingPipe(SCANNER_ADDRESS);

  // Clamp length to valid range
  if (length > MAX_PAYLOAD_SIZE) length = MAX_PAYLOAD_SIZE;
  if (length == 0) length = 1;

  bool ok = txRadio.write(data, length);
  return ok;
}
