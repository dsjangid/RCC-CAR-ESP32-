# Hardware Installation & Wiring Guide

This document describes how to wire the physical components of the APEX ESP32 RC Car. 

---

## 🛠️ Hardware Bill of Materials (BOM)

| Qty | Component | Specification | Description |
| :--- | :--- | :--- | :--- |
| 1 | **ESP32 Development Board** | DevKit v1 (30 or 38-pin version) | The microcontroller processing dashboard commands |
| 1 | **L293D Motor Driver IC** | Dual H-bridge DIP-16 chip | Handles high-current switches for DC motors |
| 2 | **DC Gear Motors & Wheels** | TT Motors (3V-6V yellow gearboxes) | Differential wheels for drive and steering |
| 1 | **HC-SR04 Sensor** | Ultrasonic distance sensor (5V) | Detects obstacles in front of the vehicle |
| 1 | **Battery Source** | 2x 18650 Li-ion batteries (7.4V - 8.4V) | Powers the motors and logic |
| 1 | **Battery Holder** | 2S 18650 slot holder | Holds batteries securely |
| 2 | **Resistors (Voltage Divider)** | 10kΩ and 4.7kΩ resistors | Safety scale down for battery sensing |
| 1 | **Slide Switch** | SPST on/off switch | Cuts power to the whole circuit |
| 1 | **Breadboard & Jumper Wires** | Solderless breadboard | Interconnects all modules |

---

## 🔌 Circuit Wiring Table

| Source Device/Pin | Destination Device/Pin | Wire Color (Rec.) | Description |
| :--- | :--- | :--- | :--- |
| **ESP32 GND** | Common GND Rail | Black | Logic ground reference |
| **ESP32 VIN (5V)** | Common 5V Rail / L293D VCC1 | Red | Powers the logic chip |
| **ESP32 3V3** | Breadboard 3.3V Rail | Orange | Reference voltage for divider |
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

## ⚙️ L293D Pinout Diagram

The L293D is a standard 16-pin H-Bridge driver. The pin layout is shown below:

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

### Motor Connection Rules
- **VCC1 (Pin 16):** Power for internal logic. Must connect to ESP32 **VIN** (5V).
- **VCC2 (Pin 8):** Power for motors. Connects directly to the **battery positive (+)** through your SPST slide switch.
- **GND (Pins 4, 5, 12, 13):** Common ground for both the ESP32 and battery negative (-).
- **Left Motor:** Connected to **1Y (Pin 3)** and **2Y (Pin 6)**.
- **Right Motor:** Connected to **3Y (Pin 11)** and **4Y (Pin 14)**.

---

## ⚡ Battery Voltage Divider (Safety Sensor)

The ESP32 analog pins can only safely accept a maximum of **3.3V**. Since your 2S battery pack outputs **8.4V** when fully charged, connecting it directly to pin **GPIO 34** will destroy the ESP32 chip.

To safely measure battery capacity, you must construct a simple voltage divider using two resistors to scale the voltage down:

```text
    Battery (+) ----[ 10k Ohm (R1) ]----+-----> To ESP32 GPIO 34 (ADC)
                                        |
                                 [ 4.7k Ohm (R2) ]
                                        |
    Common Ground ----------------------+----------------------
```

### Formula and Calculation:
$$V_{\text{out}} = V_{\text{battery}} \times \left( \frac{R_2}{R_1 + R_2} \right)$$
$$V_{\text{out}} = 8.4\text{V} \times \left( \frac{4.7\text{k}\Omega}{10\text{k}\Omega + 4.7\text{k}\Omega} \right) \approx 2.68\text{V}$$

A maximum battery voltage of 8.4V scales to approximately 2.68V, which is well within the ESP32's 3.3V safe limit. The firmware automatically maps this ADC reading back to the real voltage and battery percentage.
