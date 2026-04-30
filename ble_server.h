/*
  ble_server.h - Bluetooth Low Energy Server Interface
  ======================================================
  This sets up the ESP32 as a BLE peripheral so a phone or laptop
  can connect to it and receive captured packet data wirelessly.

  You do not need to change anything in this file.
  The BLE device name is set in config.h.
*/

#pragma once
#include <Arduino.h>
#include "nrf_handler.h"

// Initialize and start advertising the BLE service
void bleServer_init();

// Send a captured packet over BLE to all connected clients
void bleServer_sendPacket(PacketResult &pkt);

// Returns how many BLE clients are currently connected
int bleServer_clientCount();
