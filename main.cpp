/*
  RF Interceptor - Main Firmware
  ================================
  Hardware: ESP32 + 2x NRF24L01+PA+LNA
  Author: Your Name
  License: MIT

  This is the main program file. It sets up both NRF modules,
  starts the channel scanner, and handles serial output.

  If you are new to C++/Arduino code, here is a quick guide:
  - Lines starting with // are comments. They explain what the code does.
  - setup() runs once when the device powers on.
  - loop() runs over and over forever until power is removed.
  - Functions are blocks of code that do a specific task.
*/

#include <Arduino.h>
#include <SPI.h>
#include "config.h"
#include "nrf_handler.h"
#include "ble_server.h"

// ---------------------------------------------------------------------------
// Global state
// ---------------------------------------------------------------------------
static uint32_t totalPacketCount   = 0;
static uint32_t lastStatusPrint    = 0;
static bool     deviceReady        = false;

// ---------------------------------------------------------------------------
// setup() - Runs once at startup
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(500); // Give the serial port a moment to open

  Serial.println();
  Serial.println("=========================================");
  Serial.println("  RF Interceptor v1.0");
  Serial.println("  2.4 GHz Packet Sniffer + Replayer");
  Serial.println("=========================================");
  Serial.println();

  // Set up the built-in LED so we can blink it
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize both NRF24 modules
  Serial.println("[INIT] Starting NRF modules...");
  bool module1OK = nrfScanner_init();
  bool module2OK = nrfTransmitter_init();

  if (!module1OK) {
    Serial.println("[ERROR] Scanner module (NRF #1) not found.");
    Serial.println("        Check wiring: CE=GPIO4, CSN=GPIO5");
    Serial.println("        Check the 100uF capacitor is installed.");
    blinkError(3); // 3 fast blinks = module 1 missing
  } else {
    Serial.println("[OK] Scanner module ready on CH: " + String(SCANNER_CE_PIN) + "/" + String(SCANNER_CSN_PIN));
  }

  if (!module2OK) {
    Serial.println("[ERROR] Transmitter module (NRF #2) not found.");
    Serial.println("        Check wiring: CE=GPIO16, CSN=GPIO17");
    Serial.println("        Check the 100uF capacitor is installed.");
    blinkError(5); // 5 fast blinks = module 2 missing
  } else {
    Serial.println("[OK] Transmitter module ready on CE: " + String(TX_CE_PIN) + "/" + String(TX_CSN_PIN));
  }

  // Start BLE server so phones can connect
  Serial.println("[INIT] Starting Bluetooth server...");
  bleServer_init();
  Serial.println("[OK] Bluetooth name: " + String(BLE_DEVICE_NAME));

  deviceReady = (module1OK && module2OK);

  if (deviceReady) {
    Serial.println();
    Serial.println("[READY] Scanning all 125 channels. Press 'r' to replay last packet.");
    Serial.println();
    digitalWrite(LED_PIN, HIGH); // Solid LED = all good
  } else {
    Serial.println();
    Serial.println("[WARNING] One or more modules failed. Check wiring and restart.");
  }
}

// ---------------------------------------------------------------------------
// loop() - Runs continuously after setup()
// ---------------------------------------------------------------------------
void loop() {
  if (!deviceReady) {
    // Something went wrong at startup. Keep blinking and wait.
    blinkError(2);
    delay(2000);
    return;
  }

  // Scan one channel per loop iteration.
  // The scanner cycles through all 125 channels automatically.
  PacketResult result = nrfScanner_scanNextChannel();

  if (result.packetFound) {
    totalPacketCount++;

    // Blink the LED briefly to indicate a captured packet
    digitalWrite(LED_PIN, LOW);
    delay(10);
    digitalWrite(LED_PIN, HIGH);

    // Print the packet to serial monitor
    printPacket(result);

    // Send the packet over BLE so a phone can see it
    bleServer_sendPacket(result);

    // Store the last packet so we can replay it if asked
    nrfTransmitter_storeLastPacket(result);
  }

  // Check if the user typed a command in the serial monitor
  handleSerialCommands();

  // Print a status summary every 10 seconds
  if (millis() - lastStatusPrint > 10000) {
    lastStatusPrint = millis();
    Serial.println("[STATUS] Total packets captured: " + String(totalPacketCount));
    Serial.println("[STATUS] Current channel: " + String(nrfScanner_getCurrentChannel()));
    Serial.println("[STATUS] BLE clients connected: " + String(bleServer_clientCount()));
  }
}

// ---------------------------------------------------------------------------
// printPacket() - Prints a captured packet to the serial monitor
// ---------------------------------------------------------------------------
void printPacket(PacketResult &pkt) {
  Serial.print("[PKT] CH:");
  Serial.print(pkt.channel);
  Serial.print("  RSSI:");
  Serial.print(pkt.rssi);
  Serial.print("  LEN:");
  Serial.print(pkt.length);
  Serial.print("  DATA: ");

  for (int i = 0; i < pkt.length; i++) {
    if (pkt.data[i] < 0x10) Serial.print("0"); // Pad single digits
    Serial.print(pkt.data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

// ---------------------------------------------------------------------------
// handleSerialCommands() - Reads commands typed in the serial monitor
// ---------------------------------------------------------------------------
void handleSerialCommands() {
  if (!Serial.available()) return;

  char cmd = Serial.read();

  switch (cmd) {
    case 'r':
    case 'R':
      Serial.println("[CMD] Replaying last captured packet...");
      nrfTransmitter_replayLastPacket();
      break;

    case 's':
    case 'S':
      Serial.println("[CMD] Scanner status:");
      nrfScanner_printStatus();
      break;

    case 'h':
    case 'H':
      Serial.println();
      Serial.println("Available commands:");
      Serial.println("  r - Replay the last captured packet");
      Serial.println("  s - Print scanner status");
      Serial.println("  h - Show this help message");
      Serial.println();
      break;

    default:
      break;
  }
}

// ---------------------------------------------------------------------------
// blinkError() - Blinks the LED a given number of times to signal an error
// ---------------------------------------------------------------------------
void blinkError(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
    delay(150);
  }
}
