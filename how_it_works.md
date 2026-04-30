# How It Works

This document explains what is happening inside the device at a technical level. You do not need to read this to build or use the project. It is here for people who want to understand the theory and go deeper.

---

## The 2.4GHz Radio Band

The 2.4GHz ISM (Industrial Scientific Medical) band is a slice of radio spectrum that anyone can use without a license. It runs from 2.400 GHz to 2.527 GHz. Dozens of technologies share this space:

- Bluetooth Classic and BLE
- WiFi (802.11 b/g/n channels 1 through 13)
- NRF24L01 proprietary protocol (ShockBurst and Enhanced ShockBurst)
- Zigbee
- Wireless keyboards and mice
- Baby monitors
- RC controllers

The NRF24L01 divides this space into 125 channels, each 1 MHz wide. Channel 0 is at 2.400 GHz, channel 1 is at 2.401 GHz, and so on up to channel 124 at 2.524 GHz.

---

## How the NRF24L01 Receives Packets

The NRF24L01 is not a true promiscuous sniffer. It has a hardware filter based on a 3 to 5 byte address. When a packet arrives, the radio looks at the first few bytes and only passes the packet to the microcontroller if they match the configured address. This is why a basic NRF24 scanner cannot capture all traffic the way a software-defined radio (SDR) can.

What we can do instead is:
- Configure the address bytes to common patterns used by devices we want to test
- Use the carrier detect function to know when any transmission is happening on a channel
- Cycle through all channels quickly to build a map of which channels are active

The address we use in this code (0xABCDABCD71) is an example. If you want to capture packets from a specific device, you need to know or guess the address it uses and configure the scanner to match.

---

## Channel Hopping

The scanner works by rapidly switching the NRF module through all 125 channels in sequence. On each channel it waits for DWELL_TIME_US microseconds (default 256us) and then checks two things:

1. `testCarrier()` - This returns true if the radio detected any energy on the channel above a threshold, even if the packet was not addressed to us. It is useful for finding busy channels.

2. `available()` - This returns true if a complete packet was received that matched our address. This is where we get actual data.

At 256us per channel and 125 channels, a full sweep takes about 32 milliseconds. That means the scanner completes roughly 31 full sweeps per second.

---

## Why We Use Two Modules

Using two separate NRF24L01 modules connected to the same SPI bus (but with different CE and CSN pins) solves a key problem: an NRF radio can either listen or transmit at any one time, not both. If we tried to use one module for both scanning and replaying, we would have to stop scanning every time we wanted to replay, which would cause us to miss traffic.

With two modules:
- Module 1 scans continuously and never transmits
- Module 2 transmits on demand without interrupting module 1

The ESP32 keeps both modules on the same hardware SPI bus and manages them independently using their respective CSN pins.

---

## The Replay Attack Concept

When the transmitter module replays a packet, it sends the exact byte sequence the scanner captured, on the exact same channel. If the original device was sending a command (like "unlock" or "turn on"), and the receiving device does not use a rolling code or nonce, it will respond to the replayed command as if it came from the original device.

This is why rolling codes exist in car key fobs and garage doors. Each code is used only once. A replay of an old code is rejected. This device will not work against rolling-code systems but is useful for testing simple fixed-code RF devices.

---

## BLE Data Stream Format

When a phone or computer connects over BLE and enables notifications on the characteristic, the ESP32 sends a notification every time a packet is captured. The notification payload is structured as:

```
Byte 0:     Channel number (0 to 124)
Byte 1:     Payload length (1 to 32 bytes)
Byte 2:     RSSI offset value (actual RSSI = this value - 128)
Bytes 3+:   Raw packet bytes
```

The demo HTML file parses this format and displays it in a live table.

---

## Extending This Project

Some directions you could take this project further:

**True promiscuous sniffing with nRF Sniffer**
Nordic Semiconductor makes official firmware called nRF Sniffer that runs on an nRF52840 dongle and can capture all BLE traffic by following connection parameters. It integrates with Wireshark. This is a separate product from the NRF24L01 used here but is the proper tool for serious BLE analysis.

**Software Defined Radio (SDR)**
For true wideband capture of the entire 2.4GHz band, an RTL-SDR dongle with a 2.4GHz upconverter or a HackRF One can capture everything at once. This goes far beyond what the NRF24L01 can do.

**Frequency hopping detection**
Bluetooth uses frequency hopping spread spectrum (FHSS), jumping to a new channel 1600 times per second following a pseudorandom pattern. Tracking this pattern requires hardware designed for it. A simple NRF scanner can detect that Bluetooth traffic is present but cannot follow a connection.

**Power saving**
The ESP32 supports several sleep modes. Between channel scans you could put the processor into light sleep and wake on a timer or on a signal from the NRF IRQ pin. This could extend battery life from 4 hours to 10 or more hours.
