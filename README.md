# Intelligent Pothole Warning Road (Arduino Based)

An Arduino-based smart road safety model that detects vehicles approaching a pothole and automatically warns drivers using **visual indicators (Red/Green LEDs)**, an **audible Buzzer alarm**, a **motorized moving warning sign** (*"POTHOLE AHEAD — SLOW DOWN"*), and a **GME12864-4 OLED Display** showing live road status messages.

---

## How It Works

1. **Normal Road State (Clear):**
   - The **Green LED** stays **ON**.
   - The **Red LED** and **Buzzer** remain **OFF**.
   - The **SG90 Servo Motor** holds the warning sign lowered (`0°`).
   - The **GME12864-4 OLED** displays: **`ROAD STATUS: NORMAL` / `ALL CLEAR` / `Drive Safely`**.
2. **Vehicle Approaches / Pothole Area Triggered:**
   - The **IR Sensor** (mounted near/under the pothole zone) detects the approaching toy car and sends a signal to Arduino pin `D2`.
3. **Warning Alert Activated:**
   - The **Servo Motor** rotates to `90°` to **raise the warning sign**.
   - The **Green LED** turns **OFF** and the **Red LED** turns **ON**.
   - The **5V Buzzer** sounds a pulsing alarm to alert the driver.
   - The **GME12864-4 OLED** displays an inverted **`WARNING!`** banner with **`POTHOLE AHEAD! / SLOW DOWN / Vehicle Detected`**.
4. **Auto-Reset:**
   - Once the vehicle passes and the sensor remains clear for 3 seconds (`ALERT_HOLD_MS`), the servo lowers the sign back to `0°`, the Red LED and Buzzer turn **OFF**, the Green LED turns **ON**, and the OLED switches back to **`ALL CLEAR`**.

| System State | IR Sensor (`D2`) | Green LED (`D8`) | Red LED (`D7`) | Buzzer (`D6`) | Servo Sign (`D9`) | OLED Display (`A4`/`A5`) |
| :--- | :---: | :---: | :---: | :---: | :---: | :--- |
| **Normal Road** | `HIGH` (No vehicle) | **ON** | OFF | OFF | Lowered (`0°`) | `ALL CLEAR` / `Drive Safely` |
| **Pothole Alert** | `LOW` (Vehicle detected) | OFF | **ON** | **Pulsing ON/OFF** | Raised (`90°`) | `WARNING! POTHOLE AHEAD! SLOW DOWN` |

---

## List of Components

| # | Component | Quantity | Purpose |
| :---: | :--- | :---: | :--- |
| 1 | **Arduino UNO R3 / UNO Mini** | 1 | Main microcontroller processing sensor input and controlling outputs |
| 2 | **IR Obstacle Avoidance Sensor Module** | 1 | Detects the vehicle reaching the pothole zone |
| 3 | **SG90 Micro Servo Motor** | 1 | Raises and lowers the *"POTHOLE AHEAD SLOW DOWN"* sign |
| 4 | **5V Active Buzzer** | 1 | Audible alarm when a vehicle approaches the pothole |
| 5 | **GME12864-4 OLED Display (0.96" 128×64, I2C SSD1306)** | 1 | Displays live road status and pothole warning messages |
| 6 | **Red LED (5mm)** | 1 | Warning light indicator during alert state |
| 7 | **Green LED (5mm)** | 1 | Normal road status indicator |
| 8 | **220Ω Resistors** | 2 | Current-limiting resistors for the Red and Green LEDs |
| 9 | **Breadboard (Half/Full size)** | 1 | Solderless prototyping board for power distribution & LEDs |
| 10 | **Jumper Wires (M-M & M-F)** | 1 set | Connecting Arduino, breadboard, sensor, servo, LEDs, buzzer, and OLED |
| 11 | **5V Power Bank + USB Cable** | 1 | Portable 5V power supply (USB-B for UNO R3, USB-C for UNO Mini) |
| 12 | **Cardboard / Foam Board & Road Craft Materials** | 1 | Physical diorama base with road, pothole, and warning signboard |
| 13 | **Toy Car** | 1 | Demonstration vehicle for triggering the sensor |

---

## Circuit Diagram

See the full color-coded diagram in [`circuit_diagram.svg`](./circuit_diagram.svg):

![Intelligent Pothole Warning Road Circuit Diagram](./circuit_diagram.svg)

### ASCII Wiring Schematic

```text
  [5V Power Bank] ===(USB Cable)===> [Arduino UNO USB Port]

                         +-------------------+
                         |    Arduino UNO    |
                         |                   |
   IR Sensor (OUT) ----->| D2            5V  |-----> Breadboard (+) 5V Rail
                         |               GND |-----> Breadboard (-) GND Rail
   Buzzer (+)      <-----| D6                |
   Red LED (+)     <--[220Ω]-- D7            |
   Green LED (+)   <--[220Ω]-- D8        A4  |<----> GME12864-4 OLED (SDA)
   SG90 Servo SIG  <-----| D9 (PWM)      A5  |-----> GME12864-4 OLED (SCL)
                         +-------------------+

  Breadboard (+) 5V Rail  ---> IR Sensor VCC, SG90 Servo VCC (Red), OLED VCC
  Breadboard (-) GND Rail ---> IR Sensor GND, SG90 Servo GND (Brown), OLED GND,
                               Red LED (-), Green LED (-), Buzzer (-)
```

> **Note:** On a standard breadboard, connect the top and bottom `+` / `-` power rails together with two jumper wires if you use both sides, and power the Arduino UNO via its **USB port** from the 5V Power Bank (do **not** supply 5V to `VIN`, as `VIN` requires 7V–12V).

---

## Circuit Connection Table

| Component | Component Pin / Lead | Connect To | Notes |
| :--- | :--- | :--- | :--- |
| **Power Bank (5V)** | USB Output | Arduino UNO **USB Port** | Provides regulated 5V power |
| **Arduino UNO** | `5V` | Breadboard **`+` (5V) Rail** | Supplies 5V to sensor, servo & OLED |
| **Arduino UNO** | `GND` | Breadboard **`-` (GND) Rail** | Common ground reference |
| **IR Sensor** | `VCC` | Breadboard **`+` (5V) Rail** | Sensor power |
| **IR Sensor** | `GND` | Breadboard **`-` (GND) Rail** | Sensor ground |
| **IR Sensor** | `OUT` | Arduino Digital Pin **`D2`** | Active-`LOW` detection signal |
| **SG90 Servo Motor** | `VCC` (Red wire) | Breadboard **`+` (5V) Rail** | Servo power |
| **SG90 Servo Motor** | `GND` (Brown/Black wire) | Breadboard **`-` (GND) Rail** | Servo ground |
| **SG90 Servo Motor** | `SIG` (Orange/Yellow wire) | Arduino PWM Pin **`D9`** | Controls sign angle (`0°` / `90°`) |
| **Red LED (Warning)** | Anode (`+`, longer leg) | Arduino Digital Pin **`D7`** via **`220Ω` Resistor** | Turns ON during alert |
| **Red LED (Warning)** | Cathode (`-`, shorter leg) | Breadboard **`-` (GND) Rail** | Ground |
| **Green LED (Normal)** | Anode (`+`, longer leg) | Arduino Digital Pin **`D8`** via **`220Ω` Resistor** | Turns ON when road is clear |
| **Green LED (Normal)** | Cathode (`-`, shorter leg) | Breadboard **`-` (GND) Rail** | Ground |
| **5V Active Buzzer** | Positive (`+`) | Arduino Digital Pin **`D6`** | Sounds pulsing alarm on alert |
| **5V Active Buzzer** | Negative (`-`) | Breadboard **`-` (GND) Rail** | Ground |
| **GME12864-4 OLED** | `GND` | Breadboard **`-` (GND) Rail** | Display ground |
| **GME12864-4 OLED** | `VCC` | Breadboard **`+` (5V) Rail** | Accepts 3.3V–5V |
| **GME12864-4 OLED** | `SCL` | Arduino Analog Pin **`A5`** (`SCL`) | I2C clock line |
| **GME12864-4 OLED** | `SDA` | Arduino Analog Pin **`A4`** (`SDA`) | I2C data line |

---

## Running the Project

1. Open [`pothole.ino`](./pothole.ino) in the **Arduino IDE**.
2. Install the required OLED libraries via **Sketch → Include Library → Manage Libraries...**:
   - **Adafruit SSD1306** (by Adafruit)
   - **Adafruit GFX Library** (by Adafruit)
3. Go to **Tools → Board** and select **Arduino UNO** (or **Arduino UNO Mini**).
4. Go to **Tools → Port** and select your connected Arduino COM/USB port.
5. Click **Upload** (`Ctrl+U`).
6. Disconnect from PC (optional) and plug the USB cable into the **5V Power Bank**.

### Calibration & Tips
- **OLED I2C Address:** Most GME12864-4 displays use address `0x3C` (`OLED_I2C_ADDR` in `pothole.ino`). If your display remains blank, check your `A4`/`A5` wiring and try changing `OLED_I2C_ADDR` to `0x3D`.
- **IR Sensor Sensitivity:** Turn the blue potentiometer screw on the IR sensor module with a small screwdriver until it reliably triggers only when the toy car passes over the pothole zone.
- **Sensor Polarity:** Standard IR obstacle modules output `LOW` when detecting an object (`SENSOR_DETECTED = LOW` in `pothole.ino`). If yours outputs `HIGH`, change `SENSOR_DETECTED` to `HIGH`.
- **Servo Sign Angles:** Adjust `SIGN_DOWN_ANGLE` (`0`) and `SIGN_UP_ANGLE` (`90`) in `pothole.ino` to fit how your sign stick is glued to the servo horn.
- **Passive Buzzer Option:** If you use a *passive* buzzer instead of an *active* 5V buzzer, replace `digitalWrite(BUZZER_PIN, HIGH/LOW)` with `tone(BUZZER_PIN, 2000)` and `noTone(BUZZER_PIN)`.
