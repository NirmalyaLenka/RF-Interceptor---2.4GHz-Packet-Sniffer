/*
  config.h - User Settings
  =========================
  This file contains all the settings you might want to change.
  You should not need to edit any other file for basic use.

  Each setting has a comment explaining what it does and what
  values are safe to use.
*/

#pragma once

// ---------------------------------------------------------------------------
// PIN ASSIGNMENTS - Scanner (NRF Module 1)
// Change these only if you wire your module to different ESP32 pins.
// ---------------------------------------------------------------------------
#define SCANNER_CE_PIN   4    // Chip Enable for scanner module
#define SCANNER_CSN_PIN  5    // Chip Select for scanner module

// ---------------------------------------------------------------------------
// PIN ASSIGNMENTS - Transmitter (NRF Module 2)
// ---------------------------------------------------------------------------
#define TX_CE_PIN        16   // Chip Enable for transmitter module
#define TX_CSN_PIN       17   // Chip Select for transmitter module

// ---------------------------------------------------------------------------
// SPI BUS PINS - Shared between both modules
// These are the hardware SPI pins on the ESP32.
// Do not change unless you know what you are doing.
// ---------------------------------------------------------------------------
#define SPI_SCK_PIN      18
#define SPI_MOSI_PIN     23
#define SPI_MISO_PIN     19

// ---------------------------------------------------------------------------
// LED PIN
// GPIO 2 is the built-in blue LED on most ESP32 dev boards.
// ---------------------------------------------------------------------------
#define LED_PIN          2

// ---------------------------------------------------------------------------
// SCANNER SETTINGS
// ---------------------------------------------------------------------------

// How long (in microseconds) to listen on each channel before moving on.
// Lower = faster scan but may miss short packets.
// Higher = catches more packets but scans slower.
// Recommended range: 128 to 512
#define DWELL_TIME_US    256

// Set to 1 to print a dot for each channel with no traffic (very verbose)
// Set to 0 for cleaner output (only shows channels with packets)
#define PRINT_EMPTY_CHANNELS  0

// Maximum payload size in bytes (1 to 32 for NRF24L01)
#define MAX_PAYLOAD_SIZE  32

// ---------------------------------------------------------------------------
// BLUETOOTH (BLE) SETTINGS
// This is the name your device will show up as when you scan for it.
// ---------------------------------------------------------------------------
#define BLE_DEVICE_NAME  "RF-Interceptor"

// UUID for the BLE service and characteristic
// These are just random identifiers. You can leave them as-is.
#define BLE_SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// ---------------------------------------------------------------------------
// CHANNEL RANGE
// The NRF24L01 can use channels 0 to 124.
// You can narrow the scan range to focus on a specific band.
// Channel 0 = 2.400 GHz, Channel 124 = 2.524 GHz
// ---------------------------------------------------------------------------
#define CHANNEL_MIN  0
#define CHANNEL_MAX  124
