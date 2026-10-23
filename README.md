# 🌊 IoT-Based Smart Drain Rising-Water and Flood Warning System

## 📌 Overview

The **Smart Drain Rising-Water and Flood Warning System** is an IoT-based project designed to monitor water levels inside drains and provide early warnings when the water level rises.

The system uses an **ESP32 microcontroller** and a waterproof distance sensor to measure the distance between the sensor and the water surface. Based on the measured water level, it activates LEDs and a buzzer to indicate normal, warning, or critical conditions. Sensor readings can also be uploaded to ThingSpeak for remote monitoring over Wi-Fi.

## 🎯 Objectives

* Monitor rising water levels inside drains.
* Identify warning and critical water-level conditions.
* Provide local alerts using LEDs and a buzzer.
* Upload sensor readings to the ThingSpeak IoT platform.
* Demonstrate an affordable prototype for drain monitoring and flood awareness.

## 🛠️ Hardware Requirements

* ESP32 development board
* Waterproof ultrasonic distance sensor
* Red LED
* Yellow LED
* Green LED
* Buzzer
* Resistors and connecting wires
* Breadboard
* USB cable or suitable power supply

## 💻 Software and Technologies

* Arduino IDE
* Embedded C++ (Arduino framework)
* ESP32 Wi-Fi
* ThingSpeak IoT platform
* HTTP communication
* Arduino `WiFi.h` and `HTTPClient.h` libraries

## ⚙️ Working Principle

1. **Distance Measurement:** The sensor measures the distance between itself and the water surface.
2. **Water-Level Calculation:** The ESP32 converts the measured distance into an estimated water-level percentage using the configured reference height.
3. **Status Detection:** The program classifies the water level as Normal, Warning, or Critical according to predefined thresholds.
4. **Local Alert:** LEDs indicate the current status, while the buzzer provides an audible alert when configured warning conditions are reached.
5. **Cloud Monitoring:** The ESP32 sends the water-level percentage, measured distance, and status value to ThingSpeak through a Wi-Fi connection.
6. **Remote Observation:** The uploaded readings can be viewed on a ThingSpeak channel dashboard.

## 🚦 Alert Conditions

The code uses configurable thresholds to determine the water-level status.

| Status   | Condition                                  | Indication         |
| -------- | ------------------------------------------ | ------------------ |
| Normal   | Water level below the warning threshold    | Green LED          |
| Warning  | Water level reaches the warning threshold  | Yellow LED         |
| Critical | Water level reaches the critical threshold | Red LED and buzzer |

**Note:** The exact thresholds and alert behavior depend on the values configured in the source code.

## 📡 ThingSpeak Integration

The project can send the following data to ThingSpeak:

* **Field 1:** Water-level percentage
* **Field 2:** Measured distance
* **Field 3:** Numeric status code

The ESP32 uses an HTTP request to upload the readings. A ThingSpeak channel must be configured before cloud monitoring can work.

## 🔌 Pin Configuration

The following pin assignments are used in the provided program.

| Component             | ESP32 GPIO |
| --------------------- | ---------: |
| Sensor Trigger (TRIG) |          5 |
| Sensor Echo (ECHO)    |         18 |
| Green LED             |         25 |
| Yellow LED            |         26 |
| Red LED               |         27 |
| Buzzer                |         14 |

**Important:** Verify that the sensor's ECHO output voltage is safe for the ESP32's 3.3 V GPIO. Use an appropriate voltage divider or level shifter if required.

## 🚀 Setup and Installation

### 1. Install Arduino IDE

Install the Arduino IDE and configure support for the ESP32 development board.

### 2. Connect the Components

Connect the sensor, LEDs, and buzzer to the ESP32 according to the pin configuration above. Ensure all components share a suitable ground connection.

### 3. Configure Wi-Fi

Open the Arduino source file and enter your Wi-Fi credentials in the appropriate variables.

```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

### 4. Configure ThingSpeak

Create a ThingSpeak channel with three fields:

* Field 1: Water Level
* Field 2: Distance
* Field 3: Status

Copy the channel's **Write API Key** and enter it into the corresponding variable in the program.

### 5. Upload the Program

1. Connect the ESP32 to your computer using a USB cable.
2. Select the correct ESP32 board and serial port in Arduino IDE.
3. Verify that the required libraries are available.
4. Upload the program to the ESP32.
5. Open the Serial Monitor to observe the measured distance, calculated water level, and current status.

### 6. Monitor the Readings

If Wi-Fi and ThingSpeak are configured correctly, open your ThingSpeak channel dashboard to view the uploaded sensor readings.

## 🧪 Testing

The prototype can be evaluated by changing the distance between the sensor and a safe test surface representing the water level.

Check whether:

* The sensor produces reasonable distance readings.
* The calculated water-level percentage changes as the distance changes.
* The LEDs indicate the configured status correctly.
* The buzzer activates under the programmed conditions.
* ThingSpeak receives readings when Wi-Fi and API settings are correct.

**Testing results should be added after the system has actually been tested.** This repository does not claim verified field performance or real-world flood prediction accuracy.

## 🌍 Applications

* Drainage-system monitoring prototypes
* Water-level monitoring demonstrations
* Flood-awareness projects
* Smart-city and IoT academic projects
* Educational demonstrations of sensor-based alert systems

## ⚠️ Limitations

* Sensor readings may be affected by debris, turbulence, condensation, or sensor placement.
* Wi-Fi or internet failure can interrupt cloud updates.
* The prototype requires suitable installation and calibration for the target drain.
* Water-level thresholds must be adjusted for the installation.
* This prototype is **not a certified flood-warning or public-safety system**.

## 🔮 Future Enhancements

* Add SMS or mobile notifications.
* Develop a live monitoring dashboard.
* Store historical readings for trend analysis.
* Add battery backup and power monitoring.
* Improve sensor validation and fault detection.
* Test the system under controlled conditions before considering real-world deployment.

## 🔐 Security Note

Do not upload real Wi-Fi passwords or ThingSpeak Write API Keys to a public repository. Keep credentials private and use placeholders in published source code.

## 👨‍💻 Project Type

**Internet of Things (IoT) | Embedded Systems | Environmental Monitoring**

## 📄 License

Choose a suitable open-source license before distributing or reusing this project. If no license is specified, reuse permissions are not automatically granted.
