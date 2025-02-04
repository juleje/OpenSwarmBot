# OpenSwarmBot
OpenSwarmBot is a low-power, four-wheel robot controlled via MQTT and BLE communication. This application is developped for the course Industrial Internet Infrastructure for the master Computer Science. The system is designed to provide secure control and monitoring of the robot through a Raspberry Pi, a Flask-based frontend, and an MQTT broker. The robot can perform various movements, send sensor data (RPM and voltage), and log data for visualization and analysis. 

## Features
- Secure MQTT Communication: Uses TLS and authentication for safe data transmission.
- Bluetooth Low Energy (BLE): Handles robot communication efficiently.
- Web-based Control Interface: Allows remote control via a Flask-powered UI.
- Real-time Monitoring: Captures and visualizes RPM and voltage data using InfluxDB and Chronograf.
- Alert System: Sends battery status updates and alerts based on predefined thresholds.

## System Overview
### Raspberry Pi
Runs runHandler.py, which starts two communication threads:
- UART → MQTT: Reads data from the robot via UART, validates it, and publishes it to the MQTT broker.
- MQTT → UART: Listens for control messages from the MQTT broker and forwards them to the robot via UART.
### MQTT Broker
- Uses Mosquitto with TLS and authentication.
- Clients authenticate using different user accounts (rpi, frontend, tick).
- Topics used:
  - III2024/05/sense → Publishes sensor data.
  - III2024/05/control → Receives control commands.
### Frontend (Web UI)
Flask web server provides:
- Control interface: Sends movement commands via MQTT.
- Battery monitoring: Alerts when voltage drops.
### TICK Stack (Data Logging & Alerts)
- InfluxDB: Stores robot sensor data.
- Telegraf: Reads MQTT messages and forwards them to InfluxDB.
- Chronograf: Visualizes RPM and voltage data.
- Kapacitor/InfluxDB Tasks: Triggers alerts when voltage is low.
### Embedded (Robot Firmware)
- Uses BLE for communication.
- Freebot Peripheral: Collects sensor data and sends it to the BLE Central.
- BLE Central: Receives data and forwards it to the Raspberry Pi.

## Collaborators
- Jules Verbessem (r0957436)
- Mathias Van den Cruijce (r0785409)
- Stijn Hendrix (r0797253)
- Yurryt Vermeire (r0786618)
- Arthur Spillebeen (r0762529)
