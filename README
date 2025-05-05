# AVR UART and Interrupt Communication Project

This project implements UART communication with interrupt handling on an Arduino UNO/Nano with ATmega328P microcontroller.

## Hardware Setup

The following diagram shows the hardware setup for the UART communication with RS232 interface:

![Arduino UART RS232 Circuit Diagram](https://www.mermaidchart.com/raw/c948dd95-7b93-4396-bd53-db88b82ba505?theme=light&version=v0.1&format=svg)

## Project Features

- Sends a "Hello World" message from the microcontroller to PC every 1 second
- Receives characters from PC and echoes them back
- Uses interrupts for efficient UART communication
- Automatically detects start and end of data transmission

## Component Requirements

- Arduino UNO R3 or Arduino Nano (ATmega328P)
- MAX232 IC for RS232 interface
- 5 × 1μF capacitors for MAX232
- DB9 connector
- USB to Serial converter (for computers without serial ports)
- Breadboard and jumper wires

## Connection Guide

### Arduino to MAX232

- Arduino TX (Pin 1) → MAX232 T1IN (Pin 11)
- Arduino RX (Pin 0) ← MAX232 R1OUT (Pin 12)
- Arduino 5V → MAX232 VCC (Pin 16)
- Arduino GND → MAX232 GND (Pin 15)

### MAX232 to RS232 (DB9)

- MAX232 T1OUT (Pin 14) → DB9 RXD (Pin 3)
- MAX232 R1IN (Pin 13) ← DB9 TXD (Pin 2)
- MAX232 GND → DB9 GND (Pin 5)

### Capacitor Connections

- 1μF capacitor between MAX232 Pin 1 and Pin 3
- 1μF capacitor between MAX232 Pin 4 and Pin 5
- 1μF capacitor between MAX232 Pin 2 and GND
- 1μF capacitor between MAX232 Pin 6 and GND
- 1μF capacitor between MAX232 VCC and GND
