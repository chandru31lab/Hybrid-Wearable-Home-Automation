# Hybrid Wearable Edge AI Home Automation & Emergency Assistance System

> **An Edge AI and LoRa-based wearable assistive system that enables voice-controlled home automation, fall detection, manual SOS, and real-time emergency alerts for elderly and mobility-impaired individuals.**

[![Platform](https://img.shields.io/badge/Platform-ESP32-blue)](https://www.espressif.com/en/products/socs/esp32)
[![AI](https://img.shields.io/badge/AI-TinyML%20%7C%20Edge%20AI-orange)](https://www.edgeimpulse.com/)
[![Communication](https://img.shields.io/badge/Communication-LoRa-green)](https://lora-alliance.org/)
[![IoT](https://img.shields.io/badge/IoT-Blynk-purple)](https://blynk.io/)
[![Language](https://img.shields.io/badge/Language-C%2FC%2B%2B-red)](https://www.arduino.cc/)

---

## 📌 Overview

This project is a **wearable-centric assistive healthcare and smart-home system** designed primarily for **elderly individuals and people with mobility impairments**, including users who may have difficulty reaching conventional switches or accessing help during emergencies.

Unlike conventional smart-home systems that use a fixed voice assistant, this project places the **voice interface on the wearable device**, allowing the user to issue commands while moving around the home.

The system combines:

* 🎙️ **Wearable voice control**
* 🧠 **TinyML / Edge AI voice recognition**
* 🏠 **Home automation**
* 🦺 **Wearable fall detection**
* 🆘 **Manual emergency trigger**
* 📡 **LoRa communication**
* 🔔 **Local emergency alarm**
* 📱 **Blynk-based IoT monitoring**
* 🌐 **Online + offline operation**
* 💊 **Medication reminder support**

The overall architecture consists of a **Wearable Transmitter Node** and a **Main Home Controller**, communicating through LoRa.

---

# 🎯 Problem

Elderly and mobility-impaired individuals may face difficulties with:

* Operating conventional electrical switches
* Moving quickly during emergencies
* Calling caregivers after a fall
* Accessing fixed smart-home voice assistants
* Operating cloud-dependent systems during internet outages

Existing smart-home solutions are frequently dependent on internet connectivity and fixed-location interfaces. This project addresses these limitations by combining **wearable interaction, local Edge AI processing, long-range communication, and emergency detection**.

---

# 💡 Proposed Solution

The system provides a wearable assistant that stays with the user.

The wearable can:

1. Receive predefined voice commands.
2. Detect falls using an MPU6050 IMU.
3. Allow the user to manually trigger an emergency.
4. Generate a local alarm after a suspected fall.
5. Transmit commands and emergency status through LoRa.

The home controller receives the LoRa packet and can:

* Switch lights ON/OFF
* Switch the fan ON/OFF
* Activate an emergency alarm
* Update the Blynk application
* Provide remote monitoring when internet connectivity is available.

---

# 🏗️ System Architecture

```text
                 ┌──────────────────────────┐
                 │       WEARABLE NODE      │
                 │                          │
                 │  ESP32                   │
                 │    │                     │
                 │    ├── VC02 Voice Module│
                 │    ├── MPU6050 IMU       │
                 │    ├── Manual SOS Button │
                 │    ├── Cancel Button     │
                 │    ├── Local Buzzer      │
                 │    └── LoRa Transmitter  │
                 └────────────┬─────────────┘
                              │
                              │ LoRa
                              ▼
                 ┌──────────────────────────┐
                 │    MAIN HOME CONTROLLER  │
                 │                          │
                 │  ESP32                   │
                 │    │                     │
                 │    ├── LoRa Receiver     │
                 │    ├── Relay - Light 1   │
                 │    ├── Relay - Light 2   │
                 │    ├── Relay - Fan      │
                 │    └── Emergency Alarm   │
                 └────────────┬─────────────┘
                              │
                     ┌────────┴────────┐
                     │                 │
                  Offline            Online
                     │                 │
                     ▼                 ▼
              Local Control       Wi-Fi + Blynk
                                       │
                                       ▼
                              Remote Monitoring
                              & Notifications
```

---

# 🧩 Hardware Components

## Wearable Node

| Component         | Purpose                     |
| ----------------- | --------------------------- |
| ESP32             | Wearable controller         |
| VC02 Voice Module | Voice command recognition   |
| MPU6050           | Motion and fall detection   |
| LoRa Module       | Long-range communication    |
| Manual Button     | Emergency/SOS trigger       |
| Cancel Button     | Cancel suspected fall alarm |
| Buzzer            | Local emergency indication  |

## Main Controller

| Component    | Purpose                |
| ------------ | ---------------------- |
| ESP32        | Central controller     |
| LoRa Module  | Receives wearable data |
| Relay Module | Appliance switching    |
| Light 1      | Controlled appliance   |
| Light 2      | Controlled appliance   |
| Fan          | Controlled appliance   |
| Alarm        | Emergency indication   |
| Wi-Fi        | Blynk connectivity     |

---

# 🧠 Edge AI / TinyML

The voice-processing subsystem uses a **keyword-spotting approach rather than general-purpose NLP**.

The workflow is:

```text
Voice Dataset
     ↓
Data Collection
     ↓
Edge Impulse
     ↓
Feature Extraction
     ↓
TinyML Model Training
     ↓
Model Optimization
     ↓
MCU-ready Deployment
     ↓
VC02 Voice Module
     ↓
Recognized Command
     ↓
ESP32
```

The model is designed around **predefined commands**, rather than free-form conversational speech.

Examples include:

```text
Turn ON Light 1
Turn OFF Light 1

Turn ON Light 2
Turn OFF Light 2

Turn ON Fan
Turn OFF Fan

Turn ON All
Turn OFF All

Help / Emergency
```

The recognized command is represented internally by a hexadecimal command code and transmitted to the main controller.

---

# 🎙️ Wearable Voice Assistant

One of the key design concepts is that the **voice interface is wearable**.

Instead of placing a voice assistant at a fixed location:

```text
Traditional System:

User → Fixed Voice Assistant → Home Controller
```

this project uses:

```text
Proposed System:

User → Wearable Voice Assistant → LoRa → Home Controller
```

This allows the user to interact with the home automation system without having to physically reach a fixed voice-control device.

---

# 🦺 Fall Detection

The wearable uses an **MPU6050 accelerometer/gyroscope** to monitor movement.

The current prototype uses acceleration magnitude to identify a two-stage fall pattern:

```text
Normal Movement
       ↓
Free Fall
       ↓
Impact
       ↓
Local Alarm
       ↓
10-second Cancellation Window
       ↓
No Cancellation
       ↓
Emergency Alert
       ↓
LoRa
```

The acceleration magnitude is calculated from the three axes:

```text
A = √(Ax² + Ay² + Az²)
```

The current prototype uses:

```text
Free-fall threshold  = 0.6 g
Impact threshold     = 1.8 g
```

These values are prototype parameters and should be experimentally calibrated for different users and environments before real-world deployment.

---

# 🆘 Emergency Mechanisms

The wearable supports multiple emergency triggers.

### 1. Fall Detection

If a free-fall event is followed by a high-impact event:

```text
Fall → Alarm → 10 sec cancellation period → Alert
```

The user can press the **Cancel button** during the 10-second period to prevent transmission of the emergency alert.

### 2. Manual Emergency Trigger

The user can press the manual emergency button at any time.

```text
Manual SOS
    ↓
Alert = 1
    ↓
LoRa Transmission
    ↓
Home Controller
```

### 3. Voice Emergency Command

A predefined emergency voice command can also be transmitted through the wearable.

---

# 📡 LoRa Communication

LoRa is used as the communication link between the wearable and home controller.

### Current packet format

The prototype uses a **3-byte packet**:

| Byte   | Data                      |
| ------ | ------------------------- |
| Byte 0 | Voice command – High byte |
| Byte 1 | Voice command – Low byte  |
| Byte 2 | Alert status              |

Where:

```text
Alert = 0 → Normal
Alert = 1 → Emergency
```

Example:

```text
[0xA1] [0x11] [0x00]
```

represents a normal voice command.

An emergency packet uses:

```text
[Command MSB] [Command LSB] [0x01]
```

---

# 🏠 Home Automation

The main ESP32 controls three appliance channels:

```text
Light 1
Light 2
Fan
```

Voice commands received through LoRa are mapped to relay operations.

For example:

```text
0xA111 → Light 1 ON
0xA118 → Light 1 OFF

0xA119 → Light 2 ON
0xA112 → Light 2 OFF

0xA120 → Fan ON
0xA121 → Fan OFF

0xA113 → Light 1 + Light 2 ON
0xA114 → Light 1 + Light 2 OFF
```

The system also supports controlling appliances through the Blynk mobile application when online.

---

# 📱 IoT Integration

When Wi-Fi is available, the main ESP32 connects to **Blynk IoT**.

The application provides virtual controls for:

```text
V0 → Light 1
V1 → Light 2
V2 → Emergency Status
V3 → Medication Alert
V4 → Fan
```

The online mode provides:

* Remote appliance control
* Emergency status
* Mobile monitoring
* Medication alert functionality

---

# 🌐 Hybrid Online–Offline Operation

A major design objective is to prevent internet connectivity from becoming a single point of failure.

### Offline Mode

The following functions continue locally:

* Wearable voice commands
* Fall detection
* Manual emergency trigger
* LoRa communication
* Appliance control
* Local alarm

### Online Mode

When internet connectivity is available, additional functionality is enabled:

* Blynk connectivity
* Mobile appliance control
* Emergency status updates
* Medication alerts
* Remote monitoring

```text
                  Internet?
                     │
             ┌───────┴───────┐
             │               │
            YES              NO
             │               │
             ▼               ▼
        Blynk Online      Local Mode
             │               │
             └───────┬───────┘
                     ↓
             Core system remains
                 operational
```

---

# 🚨 Emergency Response

The current prototype provides local emergency indication and Blynk emergency status.

The intended complete emergency workflow is:

```text
Fall / SOS / Help
       ↓
Local Alarm
       ↓
Emergency Status
       ↓
Caregiver Notification
       ↓
Caretaker Response
       ↓
Emergency Escalation
```

The project is designed for future integration of **automated caretaker and ambulance calling**, including the Indian emergency ambulance number **108**.

> **Important:** The current published firmware activates the local alarm and Blynk emergency status; automated telephone calling is an intended extension and should not be represented as implemented unless a GSM/telephony subsystem has been added and tested.

---

# 💊 Medication Reminder

The main controller includes a Blynk medication-alert interface.

The medication alert can activate the configured alert output and provide a corresponding status through the IoT interface.

Future versions can extend this into:

* Multiple medication schedules
* Voice reminders
* Caregiver confirmation
* Medication adherence logging

---

# 🔌 Pin Configuration

## Wearable ESP32

### VC02 UART

```text
VC02 RX → GPIO 16
VC02 TX → GPIO 17
```

### MPU6050

```text
SDA → GPIO 21
SCL → GPIO 22
VCC → 3.3V
GND → GND
```

### Buttons

```text
Cancel Button → GPIO 32
Manual Button → GPIO 25
```

Both buttons use:

```cpp
INPUT_PULLUP
```

and are active LOW.

### Buzzer

The current firmware uses:

```text
Buzzer → GPIO 2
```

### LoRa

```text
SCK  → GPIO 5
MISO → GPIO 19
MOSI → GPIO 27
SS   → GPIO 18
RST  → GPIO 14
DIO0 → GPIO 26
```

Frequency:

```text
433 MHz
```

---

# 🔌 Main Controller ESP32

### Appliance Outputs

```text
Light 1 → GPIO 5
Light 2 → GPIO 18
Fan     → GPIO 19
```

### Alarm

```text
Alarm 1 → GPIO 12
Alarm 2 → GPIO 4
```

### LoRa

```text
SCK  → GPIO 25
MISO → GPIO 33
MOSI → GPIO 32
SS   → GPIO 26
RST  → GPIO 14
DIO0 → GPIO 27
```

Frequency:

```text
433 MHz
```

> **Safety:** The relay-controlled AC wiring must be properly isolated and enclosed. Do not work on mains voltage without appropriate electrical safety practices and qualified supervision.

---

# 💻 Software Requirements

Install:

* Arduino IDE
* ESP32 Arduino Core
* Edge Impulse tooling/workflow
* AI Thinker voice-module tools
* Blynk library

### Arduino Libraries

Wearable:

```text
SPI
LoRa
Wire
Adafruit MPU6050
Adafruit Unified Sensor
```

Main Controller:

```text
WiFi
WiFiClient
BlynkSimpleEsp32
SPI
LoRa
```

---

# 🚀 Getting Started

## 1. Clone the Repository

```bash
git clone https://github.com/YOUR_USERNAME/Hybrid-Wearable-Home-Automation.git
cd Hybrid-Wearable-Home-Automation
```

## 2. Install Dependencies

Open Arduino IDE and install the required ESP32 board support and libraries.

## 3. Configure the Wearable

Open:

```text
Wearable_Node/Wearable_Node.ino
```

Verify:

* GPIO assignments
* LoRa frequency
* VC02 UART pins
* MPU6050 connection
* Button connections

Upload the firmware to the wearable ESP32.

## 4. Configure the Main Controller

Open:

```text
Main_Controller/Main_Controller.ino
```

Configure your own:

```cpp
#define BLYNK_AUTH_TOKEN "YOUR_TOKEN"

char ssid[] = "YOUR_WIFI";
char pass[] = "YOUR_PASSWORD";
```

**Do not upload real Wi-Fi passwords or Blynk authentication tokens to GitHub.**

Upload the firmware to the main ESP32.

## 5. Configure Blynk

Create a Blynk template and configure the corresponding virtual pins:

```text
V0 → Light 1
V1 → Light 2
V2 → Emergency Status
V3 → Medication Alert
V4 → Fan
```

## 6. Test LoRa

Power both nodes and verify that the wearable can transmit:

```text
Voice Command
+
Alert Status
```

to the main controller.

---

# 🧪 Testing

The prototype should be evaluated under the following scenarios:

| Test                  | Expected Result                        |
| --------------------- | -------------------------------------- |
| Light 1 voice command | Light 1 switches                       |
| Light 2 voice command | Light 2 switches                       |
| Fan voice command     | Fan switches                           |
| All ON/OFF            | Corresponding appliances switch        |
| Manual SOS            | Emergency status transmitted           |
| Simulated fall        | Local alarm activates                  |
| Fall + Cancel         | Alarm stops; no emergency transmission |
| Fall without Cancel   | Alert transmitted through LoRa         |
| Wi-Fi available       | Blynk becomes available                |
| Wi-Fi unavailable     | Core local functions continue          |

---

# 📊 Reported Prototype Performance

During prototype evaluation, the project was designed around the following target/reported performance figures:

| Metric                     | Reported/Target Value |
| -------------------------- | --------------------: |
| Voice recognition accuracy |               ~92–95% |
| Voice response time        |           ~300–500 ms |
| Fall detection accuracy    |               ~90–93% |
| Operating modes            |      Online + Offline |
| Wearable communication     |                  LoRa |
| Home appliances            |      2 Lights + 1 Fan |

> Performance values should be replaced with your final measured experimental results before publication. Avoid presenting estimated values as experimentally validated results.

---

# 🧠 Why This Project Is Different

### Conventional Smart Home

```text
User
 ↓
Fixed Voice Assistant
 ↓
Internet/Cloud
 ↓
Home Automation
```

### Proposed System

```text
                 ┌── Voice
                 │
User → Wearable ─┼── Fall Detection
                 │
                 └── Manual SOS
                       ↓
                     LoRa
                       ↓
                Main Home Controller
                   ↙          ↘
              Appliances     Emergency
                  ↓              ↓
               Lights/Fan    Alarm/IoT
```

### Key Innovation

> **“The assistant is not fixed inside the home — it stays with the user.”**

---

# ⭐ Key Features

* **Wearable Voice Assistant** — Voice interaction is available directly from the wearable.
* **Edge AI / TinyML** — Local keyword spotting without cloud speech processing.
* **Offline Operation** — Core automation and emergency functions continue without internet.
* **Fall Detection** — MPU6050-based motion analysis.
* **Manual SOS** — User can trigger an emergency manually.
* **LoRa Communication** — Low-power communication between wearable and home controller.
* **Home Automation** — Controls two lights and a fan.
* **Blynk IoT** — Enables remote monitoring and control when online.
* **Medication Alerts** — Supports scheduled healthcare reminders.
* **Scalable Architecture** — Can be extended with additional health and communication modules.

---

# 🛠️ Project Structure

Recommended repository structure:

```text
Hybrid-Wearable-Home-Automation/
│
├── README.md
│
├── Wearable_Node/
│   ├── Wearable_Node.ino
│   └── libraries.txt
│
├── Main_Controller/
│   ├── Main_Controller.ino
│   └── libraries.txt
│
├── Hardware/
│   ├── Circuit_Diagram.png
│   ├── System_Architecture.png
│   └── Flowchart.png
│
├── Images/
│   ├── Prototype.jpg
│   ├── Wearable.jpg
│   └── Blynk_App.png
│
├── AI_Model/
│   ├── Dataset/
│   ├── Edge_Impulse/
│   └── Deployment/
│
├── Documentation/
│   ├── Research_Paper.pdf
│   └── Presentation.pdf
│
└── Results/
    ├── Accuracy.csv
    ├── Response_Time.csv
    └── Results_Graphs/
```

---

# 🔬 Research Contribution

This project explores the integration of several technologies into a unified assistive architecture:

1. **Wearable-centric voice interaction**
2. **TinyML-based offline keyword spotting**
3. **Wearable fall detection**
4. **LoRa-based wearable-to-home communication**
5. **Hybrid offline/online smart-home operation**
6. **Integrated emergency alert handling**

The architecture is particularly relevant to **assistive healthcare, smart homes, elderly care, and mobility-impaired users**.

---

# 🔮 Future Development

Potential extensions include:

* ❤️ Heart-rate monitoring
* 🩸 SpO₂ monitoring
* 🌡️ Body-temperature monitoring
* 📍 GPS location tracking
* 📞 GSM-based caretaker calling
* 🚑 Automated emergency-service escalation
* 🗣️ Multilingual voice commands
* 🤖 Personalized voice models
* 📊 Long-term health-data analytics
* 🏥 Hospital/telemedicine integration
* 🔋 Battery and power-management optimization

---

# ⚠️ Safety & Medical Disclaimer

This project is an **academic/research prototype** and is not currently a certified medical device or a substitute for professional medical care.

Fall-detection thresholds, emergency logic, and alert mechanisms should be validated extensively before deployment in real healthcare environments.

For AC appliance control, appropriate **electrical isolation, enclosure, fusing, grounding, and qualified supervision** are required.

---

# 🔐 Security

Never commit credentials such as:

```text
Wi-Fi passwords
Blynk authentication tokens
API keys
Private certificates
```

Use a local configuration file instead:

```cpp
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_TOKEN"

char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";
```

For a public repository, consider:

```text
config.h
```

and add it to:

```text
.gitignore
```

---

# 👥 Target Users

The system is particularly relevant to:

* Elderly individuals living alone
* Mobility-impaired users
* Paralysis patients
* Parkinson's patients
* Individuals at high risk of falls
* People requiring caregiver assistance
* Families monitoring elderly parents remotely

It can also be adapted for:

* Assisted-living facilities
* Rehabilitation centers
* Elder-care facilities
* Smart healthcare environments

---

# 🌍 Impact

The project aims to improve:

**Safety → Independence → Accessibility → Emergency Response**

By combining wearable technology with home automation, the user can interact with their environment while carrying the assistive interface with them.

---

# 👨‍💻 Authors

Ramachandru J
SRM Institute of Science and Technology

### Project

**Hybrid Wearable Edge AI Home Automation & Emergency Assistance System**

---

# 📜 License

This project can be released under the **MIT License** for educational and research use.

---

## ⭐ Project Summary

> **A wearable Edge AI assistant that stays with the user, controls the home through voice, detects falls, sends emergency alerts through LoRa, and connects to IoT services when internet connectivity is available.**

This repository is intended to make the project **reproducible for researchers, students, developers, and embedded-systems enthusiasts** while providing a foundation for future healthcare and assistive-technology development.
