/*
  ============================================================
  RF Interceptor - Arduino IDE Single File Version
  ============================================================
  Hardware Required:
    - ESP32 Development Board (any 30-pin version)
    - 2x NRF24L01 + PA + LNA modules (with external antenna)
    - 2x 100uF capacitors (one per NRF module)
    - 3.7V LiPo battery + TP4056 charger module
    - Toggle switch

  Wiring Summary:
    NRF MODULE 1 (Scanner)     NRF MODULE 2 (Transmitter)
    VCC  --> 3.3V              VCC  --> 3.3V
    GND  --> GND               GND  --> GND
    CE   --> GPIO 4            CE   --> GPIO 16
    CSN  --> GPIO 5            CSN  --> GPIO 17
    SCK  --> GPIO 18           SCK  --> GPIO 18  (same pin)
    MOSI --> GPIO 23           MOSI --> GPIO 23  (same pin)
    MISO --> GPIO 19           MISO --> GPIO 19  (same pin)
    IRQ  --> not connected     IRQ  --> not connected

  Libraries needed (install via Arduino Library Manager):
    - RF24 by TMRh20
    - RF24Network by TMRh20
    (BLE is built into the ESP32 Arduino core, no extra install needed)

  How to install libraries:
    1. Open Arduino IDE
    2. Go to Sketch > Include Library > Manage Libraries
    3. Search "RF24" and install the one by TMRh20
    4. Search "RF24Network" and install that too

  Serial monitor baud rate: 115200
  ============================================================
*/

#include <SPI.h>
#include <RF24.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ============================================================
// SETTINGS - Change these if you use different pins
// ============================================================

// NRF Module 1 - Scanner
#define SCANNER_CE_PIN   4
#define SCANNER_CSN_PIN  5

// NRF Module 2 - Transmitter
#define TX_CE_PIN        16
#define TX_CSN_PIN       17

// Built-in LED (GPIO 2 on most ESP32 boards)
#define LED_PIN          2

// How long to listen on each channel before moving on (microseconds)
// Lower = faster scan, Higher = catches more packets
#define DWELL_TIME_US    256

// Maximum packet size in bytes
#define MAX_PAYLOAD_SIZE 32

// Channel range (0 to 124 covers the full 2.4GHz band)
#define CHANNEL_MIN      0
#define CHANNEL_MAX      124

// Bluetooth device name - shows up when you scan from your phone
#define BLE_DEVICE_NAME  "RF-Interceptor"

// BLE service and characteristic UUIDs (leave these as-is)
#define BLE_SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// ============================================================
// DATA STRUCTURE - Holds one captured packet
// ============================================================
struct PacketResult {
  bool    packetFound;
  uint8_t channel;
  int8_t  rssi;
  uint8_t data[MAX_PAYLOAD_SIZE];
  uint8_t length;
  uint32_t timestamp;
};

// ============================================================
// GLOBAL OBJECTS
// ============================================================

// RF24(CE_pin, CSN_pin)
RF24 scannerRadio(SCANNER_CE_PIN, SCANNER_CSN_PIN);
RF24 txRadio(TX_CE_PIN, TX_CSN_PIN);

// BLE objects
BLEServer         *pServer         = nullptr;
BLECharacteristic *pCharacteristic = nullptr;

// The address the scanner listens on.
// NRF24 needs an address to receive packets.
const uint64_t SCANNER_ADDRESS = 0xABCDABCD71LL;

// Internal state variables
uint8_t     currentChannel    = CHANNEL_MIN;
uint32_t    channelCounts[125] = {0};
PacketResult lastPacket;
bool        hasStoredPacket   = false;
int         bleClients        = 0;
uint32_t    totalPackets      = 0;
uint32_t    lastStatusPrint   = 0;
bool        deviceReady       = false;

// ============================================================
// BLE CONNECTION CALLBACKS
// ============================================================
class ConnectionCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* s) override {
    bleClients++;
    Serial.println("[BLE] Client connected. Total: " + String(bleClients));
  }
  void onDisconnect(BLEServer* s) override {
    bleClients--;
    Serial.println("[BLE] Client disconnected. Total: " + String(bleClients));
    BLEDevice::startAdvertising(); // Keep advertising for new connections
  }
};

// ============================================================
// SETUP - Runs once when the ESP32 powers on
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("=========================================");
  Serial.println("  RF Interceptor v1.0");
  Serial.println("  2.4 GHz Packet Sniffer + Replayer");
  Serial.println("=========================================");
  Serial.println();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // --- Initialize Scanner (NRF Module 1) ---
  Serial.println("[INIT] Starting scanner module...");
  if (!scannerRadio.begin()) {
    Serial.println("[ERROR] Scanner module not found!");
    Serial.println("        Check: VCC=3.3V, CE=GPIO4, CSN=GPIO5");
    Serial.println("        Check: 100uF capacitor installed");
    Serial.println("        Check: SCK=18, MOSI=23, MISO=19");
    blinkError(3);
  } else if (!scannerRadio.isChipConnected()) {
    Serial.println("[ERROR] Scanner module not responding to SPI.");
    blinkError(3);
  } else {
    scannerRadio.setPALevel(RF24_PA_LOW);
    scannerRadio.setDataRate(RF24_2MBPS);
    scannerRadio.setAutoAck(false);
    scannerRadio.setPayloadSize(MAX_PAYLOAD_SIZE);
    scannerRadio.setChannel(CHANNEL_MIN);
    scannerRadio.openReadingPipe(1, SCANNER_ADDRESS);
    scannerRadio.startListening();
    Serial.println("[OK] Scanner module ready.");
    deviceReady = true;
  }

  // --- Initialize Transmitter (NRF Module 2) ---
  Serial.println("[INIT] Starting transmitter module...");
  if (!txRadio.begin()) {
    Serial.println("[ERROR] Transmitter module not found!");
    Serial.println("        Check: VCC=3.3V, CE=GPIO16, CSN=GPIO17");
    Serial.println("        Check: 100uF capacitor installed");
    blinkError(5);
    deviceReady = false;
  } else if (!txRadio.isChipConnected()) {
    Serial.println("[ERROR] Transmitter module not responding to SPI.");
    blinkError(5);
    deviceReady = false;
  } else {
    txRadio.setPALevel(RF24_PA_HIGH);
    txRadio.setDataRate(RF24_2MBPS);
    txRadio.setAutoAck(false);
    txRadio.setPayloadSize(MAX_PAYLOAD_SIZE);
    txRadio.stopListening();
    Serial.println("[OK] Transmitter module ready.");
  }

  // --- Initialize BLE ---
  Serial.println("[INIT] Starting Bluetooth...");
  BLEDevice::init(BLE_DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ConnectionCallbacks());

  BLEService *pService = pServer->createService(BLE_SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    BLE_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
  );
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising *pAdv = BLEDevice::getAdvertising();
  pAdv->addServiceUUID(BLE_SERVICE_UUID);
  pAdv->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.println("[OK] Bluetooth advertising as: " + String(BLE_DEVICE_NAME));

  // --- Done ---
  if (deviceReady) {
    Serial.println();
    Serial.println("[READY] Scanning all 125 channels.");
    Serial.println("        Commands: r=replay  s=status  h=help");
    Serial.println();
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.println();
    Serial.println("[WARNING] One or more modules failed. Check wiring and restart.");
  }
}

// ============================================================
// LOOP - Runs continuously forever
// ============================================================
void loop() {
  if (!deviceReady) {
    blinkError(2);
    delay(2000);
    return;
  }

  // Scan the next channel and check for a packet
  PacketResult result = scanNextChannel();

  if (result.packetFound) {
    totalPackets++;

    // Brief LED blink on capture
    digitalWrite(LED_PIN, LOW);
    delay(10);
    digitalWrite(LED_PIN, HIGH);

    printPacket(result);
    sendPacketBLE(result);

    // Save it so the user can replay it
    lastPacket = result;
    hasStoredPacket = true;
  }

  // Check for commands typed in the serial monitor
  handleSerialCommands();

  // Print a status summary every 10 seconds
  if (millis() - lastStatusPrint > 10000) {
    lastStatusPrint = millis();
    Serial.println("[STATUS] Packets captured: " + String(totalPackets)
      + "  |  Current channel: " + String(currentChannel)
      + "  |  BLE clients: " + String(bleClients));
  }
}

// ============================================================
// SCAN NEXT CHANNEL
// Switches to the next channel, waits, checks for a packet
// ============================================================
PacketResult scanNextChannel() {
  PacketResult result;
  result.packetFound = false;
  result.channel     = currentChannel;
  result.rssi        = 0;
  result.length      = 0;
  result.timestamp   = millis();

  scannerRadio.setChannel(currentChannel);
  scannerRadio.startListening();
  delayMicroseconds(DWELL_TIME_US);

  bool carrier = scannerRadio.testCarrier();

  if (scannerRadio.available()) {
    result.length = MAX_PAYLOAD_SIZE;
    scannerRadio.read(result.data, result.length);
    result.packetFound = true;
    result.rssi = carrier ? -64 : -90;
    channelCounts[currentChannel]++;
  }

  // Move to next channel, wrapping around at 124
  currentChannel++;
  if (currentChannel > CHANNEL_MAX) {
    currentChannel = CHANNEL_MIN;
  }

  return result;
}

// ============================================================
// PRINT PACKET to serial monitor
// ============================================================
void printPacket(PacketResult &pkt) {
  Serial.print("[PKT] CH:");
  Serial.print(pkt.channel);
  Serial.print("  RSSI:");
  Serial.print(pkt.rssi);
  Serial.print("  LEN:");
  Serial.print(pkt.length);
  Serial.print("  DATA: ");

  for (int i = 0; i < pkt.length; i++) {
    if (pkt.data[i] < 0x10) Serial.print("0");
    Serial.print(pkt.data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

// ============================================================
// SEND PACKET over Bluetooth to connected phone
// ============================================================
void sendPacketBLE(PacketResult &pkt) {
  if (bleClients == 0) return;

  // Format: [channel, length, rssi+128, data bytes...]
  uint8_t payload[3 + MAX_PAYLOAD_SIZE];
  payload[0] = pkt.channel;
  payload[1] = pkt.length;
  payload[2] = (uint8_t)((int)pkt.rssi + 128);

  uint8_t len = 3;
  for (int i = 0; i < pkt.length && i < MAX_PAYLOAD_SIZE; i++) {
    payload[len++] = pkt.data[i];
  }

  pCharacteristic->setValue(payload, len);
  pCharacteristic->notify();
}

// ============================================================
// REPLAY LAST PACKET using the transmitter module
// ============================================================
void replayLastPacket() {
  if (!hasStoredPacket) {
    Serial.println("[TX] No packet stored yet. Capture one first.");
    return;
  }

  txRadio.stopListening();
  txRadio.setChannel(lastPacket.channel);
  txRadio.openWritingPipe(SCANNER_ADDRESS);

  uint8_t len = lastPacket.length;
  if (len > MAX_PAYLOAD_SIZE) len = MAX_PAYLOAD_SIZE;
  if (len == 0) len = 1;

  bool ok = txRadio.write(lastPacket.data, len);

  if (ok) {
    Serial.print("[TX] Replayed packet on channel ");
    Serial.println(lastPacket.channel);
  } else {
    Serial.println("[TX] Replay failed. Check transmitter module wiring.");
  }
}

// ============================================================
// PRINT CHANNEL STATUS TABLE
// ============================================================
void printChannelStatus() {
  Serial.println();
  Serial.println("Channels with captured packets:");
  Serial.println("CH   FREQ(MHz)  PACKETS");
  Serial.println("---  ---------  -------");

  bool any = false;
  for (int ch = CHANNEL_MIN; ch <= CHANNEL_MAX; ch++) {
    if (channelCounts[ch] > 0) {
      Serial.printf("%3d  %4d       %lu\n", ch, 2400 + ch, channelCounts[ch]);
      any = true;
    }
  }

  if (!any) {
    Serial.println("  No packets captured yet.");
  }
  Serial.println();
}

// ============================================================
// HANDLE SERIAL COMMANDS
// Type these in the serial monitor (baud 115200)
// ============================================================
void handleSerialCommands() {
  if (!Serial.available()) return;

  char cmd = Serial.read();

  switch (cmd) {
    case 'r':
    case 'R':
      Serial.println("[CMD] Replaying last captured packet...");
      replayLastPacket();
      break;

    case 's':
    case 'S':
      printChannelStatus();
      break;

    case 'h':
    case 'H':
      Serial.println();
      Serial.println("Commands:");
      Serial.println("  r = Replay the last captured packet");
      Serial.println("  s = Show channel activity table");
      Serial.println("  h = Show this help text");
      Serial.println();
      break;

    default:
      break;
  }
}

// ============================================================
// BLINK ERROR - Flashes the LED a set number of times
// Used to indicate startup errors without needing the monitor
// 3 blinks = scanner module missing
// 5 blinks = transmitter module missing
// ============================================================
void blinkError(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
    delay(150);
  }
}
