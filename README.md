# APEX ESP32 RC Car & Telemetry Dashboard

An interactive, web-controlled robotic RC Car powered by the **ESP32** microcontroller. This project integrates H-Bridge motor controls (L293D), ultrasonic distance sensors with autonomous collision prevention, and live telemetry tracking.

The vehicle can be fully simulated in **Wokwi** or built with real hardware, and is controlled over the internet using a custom laptop dashboard client communicating via **MQTT over WebSockets**.

---

## 🚀 Quick Navigation

- 🔌 **Detailed Hardware Guide:** For physical assembly instructions, schematic connections, and safety voltage divider circuits, see [HARDWARE.md](HARDWARE.md) (or [local link](file:///Users/meydivyansh/Projects/esp32-rc-car/HARDWARE.md)).
- 🖥️ **Web Dashboard UI:** View the local dashboard page [index.html](index.html) (or [local link](file:///Users/meydivyansh/Projects/esp32-rc-car/index.html)).
- 🧠 **ESP32 Firmware:** Check out the microcontroller source code [sketch.ino](sketch.ino) (or [local link](file:///Users/meydivyansh/Projects/esp32-rc-car/sketch.ino)).

---

## 📋 Project Architecture

```text
                  +-----------------------------------------+
                  |            Laptop Dashboard             |
                  |  - Keyboard WASD Controller             |
                  |  - Live Chart.js Battery Voltage Graph  |
                  |  - Obstacle Alarm & Speedometer needle  |
                  +--------------------+--------------------+
                                       | (MQTT WebSockets)
                                       v
                             +-------------------+
                             | Public Broker     |
                             | broker.hivemq.com |
                             +---------+---------+
                                       | (MQTT TCP)
                                       v
                  +--------------------+--------------------+
                  |       ESP32 Microcontroller (Wokwi)     |
                  |  - Receives drive controls              |
                  |  - Computes speed physics / ramps       |
                  |  - Monitors battery (ADC Potentiometer)  |
                  |  - Calculates obstacle proximity        |
                  +-----------------------------------------+
```

---

## 🛠️ Key System Features

### 1. Velocity Engine with Inertia Physics
The ESP32 firmware features a built-in virtual physics model. When you steer or accelerate, the dashboard's circular speedometer gauge reacts with realistic inertia (smoothing the dial needle) instead of jumping instantly, replicating a real vehicle.

### 2. Autonomous Collision Avoidance
Equipped with an ultrasonic distance sensor loop. If the car is driving forward and detects an obstacle within **15 cm**, it triggers an emergency automatic brake, updates the dashboard status to `CRITICAL STOP`, and blocks further forward input until the obstacle is cleared.

### 3. Real-Time Battery Telemetry & Charting
A dedicated battery measurement sensor (simulated via an analog potentiometer or scaled with resistors on physical boards) sends real-time voltage ratings. The web cockpit plots these points on a live Chart.js line graph and updates a dynamic battery cell graphic that flashes red under 20%.

---

## 💻 Firmware Setup & Flashing

1. Install **Arduino IDE** on your computer.
2. Go to **Tools** > **Board** > **Boards Manager**, search for `esp32` (by Espressif Systems), and install it.
3. Install the following libraries via **Sketch** > **Include Library** > **Manage Libraries**:
   - `PubSubClient` (by Nick O'Leary) - for MQTT connection.
   - `ArduinoJson` (by Benoit Blanchon) - for parsing controls and structuring telemetry data.
4. If building physical hardware, open `sketch.ino` and replace the SSID (`"Wokwi-GUEST"`) and password with your home Wi-Fi credentials.
5. Connect your ESP32 to your computer using a USB cable and upload the sketch.

---

## 🎮 Running the Simulation & Dashboard

1. Double-click the local dashboard: [index.html](file:///Users/meydivyansh/Projects/esp32-rc-car/index.html) to open it in your browser.
2. Click **CONNECT SYSTEM** (connects your cockpit to the MQTT broker).
3. Start the Wokwi simulation (using the circuit files `diagram.json`, `sketch.ino`, and `libraries.txt`).
4. Once the serial monitor prints `CONNECTED to broker!`, return to the dashboard.
5. Click inside the cockpit window to focus, and use **WASD** or **Arrow keys** to drive the car!
6. Slide the potentiometer in Wokwi to watch battery voltage readings update live on your telemetry charts.
