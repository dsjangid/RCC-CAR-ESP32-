# APEX ESP32 RC Car & Telemetry Dashboard

An interactive, web-controlled RC Car built on the **ESP32** microcontroller. This project integrates differential motor driving (L293D H-Bridge), ultrasonic distance telemetry with autonomous emergency braking, and a simulated 2S battery voltage monitor. 

The car can be simulated online in **Wokwi** or assembled as physical hardware, and is controlled via a custom local web dashboard communicating over **MQTT (WebSockets)**.

---

## 📋 Features

- 🎮 **Real-time Web Cockpit:** Drive the car using keyboard controls (`W`, `A`, `S`, `D` or arrow keys).
- 📈 **Telemetry Analytics:** Live graph of battery voltage history (Chart.js) and distance telemetry.
- ⚡ **Physics Speedometer:** Simulated speed values featuring acceleration and deceleration physics (inertia needle).
- 🛑 **Collision Avoidance:** Autonomous emergency braking if an obstacle is detected within 15 cm.
- 🔋 **Battery Monitoring:** Reads battery status via an analog pin (simulating a 2S Li-ion pack, 6.0V - 8.4V).

---

## 🛠️ Hardware Bill of Materials (BOM)

To build this project physically, you will need:

| Qty | Component | Specification |
| :--- | :--- | :--- |
| 1 | **ESP32 Development Board** | DevKit v1 (30 or 38-pin version) |
| 1 | **L293D Motor Driver IC** | Dual H-bridge DIP-16 chip (or L293D module) |
| 2 | **DC Gear Motors & Wheels** | TT Motors (3V-6V yellow gearboxes) |
| 1 | **HC-SR04 Sensor** | Ultrasonic distance sensor (5V) |
| 1 | **Battery Source** | 2x 18650 Li-ion batteries (7.4V nominal, 8.4V max) with holder |
| 2 | **Resistors (Voltage Divider)** | 10kΩ and 4.7kΩ resistors (for battery level sensing safely) |
| 1 | **Breadboard & Wires** | Half-size breadboard and male-to-male + male-to-female jumper wires |
| 1 | **Slide Switch** | Single Pole Single Throw (SPST) on/off switch |

---

## 🔌 Hardware Installation & Wiring Guide

> [!WARNING]
> **DO NOT** connect the 7.4V/8.4V battery directly to any ESP32 GPIO pin. ESP32 pins are **3.3V max tolerant**. You must use a voltage divider (explained below) to step down the battery measurement voltage safely.

### 1. Pin Connection Table

| Source Device/Pin | Destination Device/Pin | Wire Color (Rec.) | Description |
| :--- | :--- | :--- | :--- |
| **ESP32 GND** | Common GND Rail | Black | Logic ground reference |
| **ESP32 VIN (5V)** | Common 5V Rail / L293D VCC1 | Red | Powering logic |
| **ESP32 3V3** | Breadboard 3.3V Rail | Orange | Reference voltage |
| **ESP32 D12** | L293D Pin 1 (1,2EN) | Yellow | Left Motor Speed (PWM) |
| **ESP32 D13** | L293D Pin 2 (1A) | Green | Left Motor Direction 1 |
| **ESP32 D14** | L293D Pin 7 (2A) | Blue | Left Motor Direction 2 |
| **ESP32 D27** | L293D Pin 9 (3,4EN) | Yellow | Right Motor Speed (PWM) |
| **ESP32 D26** | L293D Pin 10 (3A) | Green | Right Motor Direction 1 |
| **ESP32 D25** | L293D Pin 15 (4A) | Blue | Right Motor Direction 2 |
| **ESP32 D33** | HC-SR04 TRIG | White/Cyan | Trigger pulse signal |
| **ESP32 D32** | HC-SR04 ECHO | Purple | Echo back signal |
| **ESP32 D34 (ADC)** | Voltage Divider output (V_out) | Blue | Battery measurement voltage |

---

### 2. Motor Driver (L293D) Wiring details

The L293D has a notch indicating the top. Pin numbers count down the left side (1 to 8) and back up the right side (9 to 16).

```text
               L293D PINOUT
             +------\_/------+
      1,2EN  | 1          16 |  VCC1 (Logic 5V from ESP32 VIN)
         1A  | 2          15 |  4A
         1Y  | 3          14 |  4Y   --> Connected to Motor Right Pin 2
        GND  | 4          13 |  GND  --- Common Ground
        GND  | 5          12 |  GND  --- Common Ground
         2Y  | 6          11 |  3Y   --> Connected to Motor Right Pin 1
         2A  | 7          10 |  3A
       VCC2  | 8           9 |  3,4EN
             +---------------+
```

- **VCC1 (Pin 16):** Connect to ESP32 **VIN** (5V).
- **VCC2 (Pin 8):** Connect to battery **Positive (+)** (7.4V/8.4V) via the slide switch. This supplies the power to run the motors.
- **GND (Pins 4, 5, 12, 13):** Connect all together to the common Ground rail.
- **Left Motor:** Connected to **1Y (Pin 3)** and **2Y (Pin 6)**.
- **Right Motor:** Connected to **3Y (Pin 11)** and **4Y (Pin 14)**.

---

### 3. Battery Voltage Divider Design (Critical Step)

To measure the 8.4V (fully charged) battery, we need to scale the voltage down to **2.7V** (so it fits safely under the ESP32's 3.3V ADC range). 

We create a voltage divider using two resistors:
- **$R_1$ (Connected to Battery +):** $10\text{ k}\Omega$
- **$R_2$ (Connected to Ground):** $4.7\text{ k}\Omega$

```text
    Battery (+) ----[ 10k Ohm (R1) ]----+-----> To ESP32 GPIO 34 (ADC)
                                        |
                                 [ 4.7k Ohm (R2) ]
                                        |
    Common Ground ----------------------+----------------------
```

**Formula:**
$$V_{\text{out}} = V_{\text{battery}} \times \left( \frac{R_2}{R_1 + R_2} \right)$$
$$V_{\text{out}} = 8.4\text{V} \times \left( \frac{4.7}{10 + 4.7} \right) \approx 2.68\text{V} \quad \text{(Safe for ESP32!)}$$

*Note: In the Wokwi simulation diagram (`diagram.json`), we use a Potentiometer connected to 3.3V to act as a simulator control knob for the battery voltage.*

---

## 💻 Firmware Setup & Flashing

1. Install **Arduino IDE** on your computer.
2. In Arduino IDE, go to **Tools** > **Board** > **Boards Manager**, search for `esp32` (by Espressif Systems), and install it.
3. Go to **Sketch** > **Include Library** > **Manage Libraries** and install the following dependencies:
   - **PubSubClient** (by Nick O'Leary) - for MQTT messaging.
   - **ArduinoJson** (by Benoit Blanchon) - for structured data packaging.
4. Connect your ESP32 to your laptop using a micro-USB (or USB-C) cable.
5. In Arduino IDE, select your board model (e.g., `ESP32 Dev Module`) and your USB Port.
6. Open `sketch.ino`, configure your WiFi network credentials if flashing to physical hardware (replace `"Wokwi-GUEST"` with your home WiFi SSID and password).
7. Click **Upload**.

---

## 🎮 How to Control and Drive

1. Double-click the local web dashboard file: [index.html](file:///Users/meydivyansh/Projects/esp32-rc-car/index.html) in your browser.
2. Ensure you have configured the dashboard to connect to the correct MQTT broker host (`broker.hivemq.com`) and matching base topic (`rccar/meydivyansh`).
3. Click **CONNECT SYSTEM** on the dashboard.
4. Run the ESP32 (either turn on the physical RC Car switch or click Play in Wokwi).
5. Click anywhere on the dashboard window to focus, and use **WASD** or **Arrow keys** to drive the car!
6. Adjust the potentiometer to watch the battery levels decay or rise.
