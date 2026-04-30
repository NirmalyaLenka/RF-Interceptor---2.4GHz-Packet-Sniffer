# Hardware Guide

This file covers the physical build in detail, including wiring, the battery system, and tips for putting it in an enclosure.

---

## NRF24L01 Pin Reference

The NRF24L01 module has 8 pins arranged in a 2x4 header. Looking at the module from the top with the antenna pointing away from you, the pins are arranged like this:

```
  [ ANTENNA SIDE ]

  GND   VCC
  CE    CSN
  SCK   MOSI
  MISO  IRQ

  [ USB / PCB SIDE ]
```

This pin layout is the same on both the small square module and the larger PA+LNA module with the external antenna. Always double-check with a multimeter if you are unsure.

---

## Full Wiring Table

### NRF Module 1 (Scanner) to ESP32

| NRF Pin | ESP32 Pin | Notes |
|---------|-----------|-------|
| GND | GND | Any GND pin on ESP32 |
| VCC | 3V3 | Must be 3.3V, NOT 5V |
| CE | GPIO 4 | Can be changed in config.h |
| CSN | GPIO 5 | Can be changed in config.h |
| SCK | GPIO 18 | Fixed hardware SPI pin |
| MOSI | GPIO 23 | Fixed hardware SPI pin |
| MISO | GPIO 19 | Fixed hardware SPI pin |
| IRQ | Not connected | Leave floating |

### NRF Module 2 (Transmitter) to ESP32

| NRF Pin | ESP32 Pin | Notes |
|---------|-----------|-------|
| GND | GND | Same GND rail as module 1 |
| VCC | 3V3 | Same 3V3 rail as module 1 |
| CE | GPIO 16 | Different from module 1 |
| CSN | GPIO 17 | Different from module 1 |
| SCK | GPIO 18 | Same as module 1 |
| MOSI | GPIO 23 | Same as module 1 |
| MISO | GPIO 19 | Same as module 1 |
| IRQ | Not connected | Leave floating |

The SCK, MOSI, and MISO wires are shared between both modules on the same SPI bus. The ESP32 tells them apart using the CSN (Chip Select Not) pin. When CSN is pulled LOW, that module is active. When it is HIGH, that module ignores the SPI bus.

---

## Capacitor Placement

Place a 100uF electrolytic capacitor between VCC and GND of each NRF module. Put it as physically close to the module as possible, ideally within 1 cm.

Why this matters: The NRF24L01 PA+LNA version pulls current spikes of up to 115mA when the transmitter fires. Without the capacitor, the voltage on the 3.3V rail drops momentarily and the module either fails to initialize or resets mid-operation.

If you cannot find 100uF capacitors, you can use:
- Two 47uF capacitors in parallel (equivalent to 94uF, close enough)
- One 220uF capacitor (more filtering, no downside)
- A 10uF ceramic capacitor in parallel with a 100uF electrolytic (best option)

---

## Battery System

The battery system has three parts: the LiPo cell, the TP4056 charge/protection module, and a power switch.

```
  LiPo cell
     |
     +---> TP4056 (BAT+ and BAT-)
               |
            OUT+ and OUT- (protected output)
               |
            Toggle switch on OUT+
               |
            ESP32 VIN pin
               |
            ESP32 GND to OUT-
```

The TP4056 module with protection circuit is important. It protects the battery from:
- Overcharge (stops charging when full)
- Over-discharge (cuts off power when battery gets too low)
- Short circuit protection

When you buy the TP4056 module, make sure it has at least 4 components on the board. The bare TP4056 module with only 2 chips lacks the protection circuit.

To charge the battery, plug a USB micro or USB-C cable (depending on your TP4056 module) into the charging port on the TP4056. The red LED on the module lights up while charging. The blue LED lights up when charging is complete. The ESP32 can remain powered on while charging.

---

## Suggested Enclosure Layout

If you are 3D printing a box or using an off-the-shelf project box:

- Mount the ESP32 near one end so the USB programming port is accessible through a hole in the side
- Mount the TP4056 near the other end with its USB port also accessible
- Place the toggle switch on the top or side
- Mount both NRF modules flat with their antennas pointing up through slots in the lid, or with SMA connectors going through the enclosure wall
- Secure the LiPo battery with double-sided tape or a strap

Minimum useful enclosure size: approximately 100mm x 60mm x 30mm

---

## Parts List With Notes

**ESP32 Development Board**
Any 30-pin or 38-pin ESP32 board works. Popular options include the ESP32 DevKitC from Espressif, or generic clones from AZ-Delivery, HiLetgo, or DOIT. Avoid boards labeled ESP32-S2, ESP32-S3, or ESP32-C3 as the pin numbers are different.

**NRF24L01 + PA + LNA Module**
Buy the version with the external antenna (a black cylindrical stub or a small whip antenna). It connects via a U.FL or SMA connector. Avoid the tiny square version without an antenna for this project as it has much shorter range. Search for "NRF24L01 + PA LNA SMA" on Amazon or AliExpress.

**TP4056 Lithium Charging Module with Protection**
Search for "TP4056 with protection" or "TP4056 DW01". The model number on the chip should include DW01 or FS8205 for the protection circuit. Cost is usually under $2 for a pack of 5.

**3.7V LiPo Battery**
Any single-cell 3.7V LiPo battery with a JST-PH 2.0 connector works. Common sizes that fit well in a small box: 603040 (900mAh), 503450 (1100mAh), 604060 (1800mAh). Do not use batteries recovered from old phones without verifying they are healthy first.

**100uF Electrolytic Capacitors**
Standard THT (through-hole) type, 10V or 16V rating, any brand. Search "100uF 16V electrolytic capacitor". Buy a pack of 10 as they are inexpensive.
