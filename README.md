# IoT Sensor Hub

## Project Description
IoT Sensor Hub is a robust IoT solution that integrates an ESP8266 and an ESP32-C6 to create a scalable sensor monitoring system. The ESP8266 serves as the primary communication module, initially acting as an Access Point (AP) to host a captive portal. This portal enables users to select a Wi-Fi network, input credentials, and configure MQTT settings for ThingsBoard integration. After configuration, the ESP8266 connects to the specified Wi-Fi and MQTT broker to publish sensor data. The ESP32-C6 interfaces with multiple sensors (motion, light, sound, IR transmitter, temperature, and humidity) and communicates readings to the ESP8266 via UART for MQTT publishing. Both modules support Over-The-Air (OTA) updates through ThingsBoard, ensuring seamless firmware maintenance.

## Features
- **Wi-Fi Configuration**: ESP8266 hosts a captive portal to scan and connect to nearby Wi-Fi networks.
- **MQTT Integration**: Publishes sensor data to ThingsBoard for real-time monitoring.
- **Sensor Suite**: ESP32-C6 interfaces with motion, light, sound, IR, temperature, and humidity sensors.
- **Serial Communication**: UART-based data transfer between ESP32-C6 and ESP8266.
- **OTA Updates**: Firmware updates for both ESP8266 and ESP32-C6 via ThingsBoard.
- **User-Friendly Setup**: Intuitive captive portal for easy configuration.

## Hardware Requirements
- ESP8266 (e.g., NodeMCU or similar)
- ESP32-C6 (e.g., ESP32-C6-DevKit)
- Sensors:
  - Motion sensor (e.g., PIR)
  - Light sensor (e.g., LDR or BH1750)
  - Sound sensor (e.g., analog microphone module)
  - IR transmitter (e.g., IR LED with driver)
  - Temperature and humidity sensor (e.g., DHT11/DHT22)
- Jumper wires and breadboard for connections
- USB cables for programming and power

## Software Requirements
- Arduino IDE or PlatformIO for programming
- Required libraries:
  - `ESP8266WiFi`, `ESP8266WebServer` (for ESP8266 AP and captive portal)
  - `PubSubClient` (for MQTT communication)
  - `ArduinoOTA` (for OTA updates)
  - Sensor-specific libraries (e.g., `DHT` for DHT sensors)
  - `EspSoftwareSerial` (for UART communication on ESP32-C6)
- ThingsBoard account for MQTT broker setup
- Arduino core for ESP8266 and ESP32 (install via Arduino IDE or PlatformIO)

## Setup Instructions
1. **Hardware Setup**:
   - Connect the ESP32-C6 to the sensors as per their pinout requirements.
   - Establish UART communication between ESP8266 and ESP32-C6 (connect TX/RX pins, ensuring proper voltage levels).
   - Power both modules via USB or external 3.3V/5V sources.

2. **Software Setup**:
   - Install the Arduino IDE or PlatformIO.
   - Add ESP8266 and ESP32 board support through the Boards Manager.
   - Install the required libraries listed above.
   - Clone or download the project repository (ensure the code is uploaded to the correct boards).

3. **Configuration**:
   - Flash the ESP8266 with the captive portal and MQTT code.
   - Flash the ESP32-C6 with the sensor reading and UART transmission code.
   - Power on the ESP8266; it will start in AP mode (default SSID: `IoT_Sensor_Hub`).
   - Connect to the AP via a device, and a captive portal will open.
   - Select a Wi-Fi network, enter credentials, and input ThingsBoard MQTT credentials (broker address, port, device token).
   - Save the settings; the ESP8266 will reboot and connect to the Wi-Fi and MQTT broker.

4. **Operation**:
   - The ESP32-C6 collects data from connected sensors.
   - Sensor data is sent to the ESP8266 via UART.
   - The ESP8266 publishes the data to ThingsBoard via MQTT.
   - Monitor the data on the ThingsBoard dashboard.

5. **OTA Updates**:
   - Configure ThingsBoard for OTA updates (upload firmware binaries to the platform).
   - Trigger OTA updates for ESP8266 and ESP32-C6 through the ThingsBoard interface.

## Project Structure
- `esp8266/`: Contains code for the ESP8266 (captive portal, Wi-Fi, MQTT, OTA).
- `esp32c6/`: Contains code for the ESP32-C6 (sensor reading, UART communication).
- `docs/`: Additional documentation and schematics (if applicable).

## Usage
- After setup, the system automatically collects and publishes sensor data to ThingsBoard.
- Use the ThingsBoard dashboard to visualize sensor readings (e.g., temperature, humidity, motion events).
- Trigger IR commands or monitor environmental changes as needed.
- Perform OTA updates to keep the firmware up-to-date without physical access.

## Troubleshooting
- **Captive Portal Not Appearing**: Ensure the ESP8266 is in AP mode and the device is connected to the correct SSID.
- **MQTT Connection Issues**: Verify ThingsBoard credentials and broker availability. Check Wi-Fi signal strength.
- **Sensor Data Not Received**: Confirm UART connections and correct pin assignments in the ESP32-C6 code.
- **OTA Update Fails**: Ensure the firmware binary is correctly uploaded to ThingsBoard and the devices are online.

## Contributing
Contributions are welcome! Please fork the repository, make changes, and submit a pull request. Ensure code follows the project’s coding style and includes relevant documentation.

## License
This project is licensed under the MIT License. See the `LICENSE` file for details.