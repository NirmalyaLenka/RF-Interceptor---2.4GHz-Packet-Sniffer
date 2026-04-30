/*
  ble_server.cpp - Bluetooth Low Energy Server Implementation
  ============================================================
  The ESP32 acts as a BLE server with one service and one characteristic.
  The characteristic sends notifications to connected clients every time
  a new packet is captured.

  The data format sent over BLE is:
    Byte 0:     Channel number (0-124)
    Byte 1:     Packet length in bytes
    Byte 2:     RSSI (cast to unsigned, add 128 to offset)
    Bytes 3+:   Raw packet data (up to 32 bytes)
*/

#include "ble_server.h"
#include "config.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ---------------------------------------------------------------------------
// Internal BLE objects
// ---------------------------------------------------------------------------
static BLEServer         *pServer         = nullptr;
static BLECharacteristic *pCharacteristic = nullptr;
static int                connectedClients = 0;

// ---------------------------------------------------------------------------
// BLE connection event callbacks
// ---------------------------------------------------------------------------
class ConnectionCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* s) override {
    connectedClients++;
    Serial.println("[BLE] Client connected. Total: " + String(connectedClients));
  }

  void onDisconnect(BLEServer* s) override {
    connectedClients--;
    Serial.println("[BLE] Client disconnected. Total: " + String(connectedClients));
    // Restart advertising so new clients can connect
    BLEDevice::startAdvertising();
  }
};

// ---------------------------------------------------------------------------
// bleServer_init()
// ---------------------------------------------------------------------------
void bleServer_init() {
  BLEDevice::init(BLE_DEVICE_NAME);

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ConnectionCallbacks());

  // Create the service
  BLEService *pService = pServer->createService(BLE_SERVICE_UUID);

  // Create the characteristic that will send packet data
  pCharacteristic = pService->createCharacteristic(
    BLE_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_READ
  );

  // Add the descriptor that clients need to enable notifications
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising so devices can find us
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  BLEDevice::startAdvertising();
}

// ---------------------------------------------------------------------------
// bleServer_sendPacket()
// ---------------------------------------------------------------------------
void bleServer_sendPacket(PacketResult &pkt) {
  if (connectedClients == 0) return; // No point sending if nobody is connected
  if (!pkt.packetFound) return;

  // Build the notification payload
  // Format: [channel, length, rssi_offset, data...]
  uint8_t payload[3 + MAX_PAYLOAD_SIZE];
  payload[0] = pkt.channel;
  payload[1] = pkt.length;
  payload[2] = (uint8_t)((int)pkt.rssi + 128); // Offset so it fits in a byte

  uint8_t payloadLen = 3;
  for (int i = 0; i < pkt.length && i < MAX_PAYLOAD_SIZE; i++) {
    payload[payloadLen++] = pkt.data[i];
  }

  pCharacteristic->setValue(payload, payloadLen);
  pCharacteristic->notify();
}

// ---------------------------------------------------------------------------
// bleServer_clientCount()
// ---------------------------------------------------------------------------
int bleServer_clientCount() {
  return connectedClients;
}
