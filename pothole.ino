/*
 * Intelligent Pothole Warning Road (Arduino Based)
 * ------------------------------------------------
 * Uses TWO IR obstacle sensors (one on each side of the pothole) to detect
 * approaching vehicles from either direction and warn drivers with:
 *   - Red Warning LED (ON) & Green Normal LED (OFF)
 *   - Pulsing 5V Buzzer alarm
 *   - SG90 Servo Motor turning the warning sign +90 deg (Side 1) or -90 deg (Side 2)
 *     relative to its 90 deg center resting position
 *   - GME12864-4 (SSD1306 128x64 I2C) OLED Display showing live status & direction
 *
 * Servo Angle Mapping (SG90 supports 0-180 degrees):
 *   - Center / Normal : 90 deg  (neutral position)
 *   - Side 1 (+90)    : 180 deg (faces Side 1 approach)
 *   - Side 2 (-90)    : 0 deg   (faces Side 2 approach)
 *
 * Required Libraries (install via Arduino IDE Library Manager):
 *   - Adafruit SSD1306
 *   - Adafruit GFX Library
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

// -------------------- OLED Configuration -----------------
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define OLED_I2C_ADDR  0x3C  // Default I2C address for GME12864-4 (change to 0x3D if needed)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool oledReady = false;

// -------------------- Pin Definitions --------------------
const int IR_SENSOR_1_PIN = 2;  // Side 1 IR Sensor OUT pin
const int IR_SENSOR_2_PIN = 3;  // Side 2 IR Sensor OUT pin
const int BUZZER_PIN      = 6;  // 5V Active Buzzer (+) pin
const int RED_LED_PIN     = 7;  // Red LED (+) via 220-ohm resistor
const int GREEN_LED_PIN   = 8;  // Green LED (+) via 220-ohm resistor
const int SERVO_PIN       = 9;  // SG90 Servo signal (orange) wire
// Note: GME12864-4 OLED uses I2C pins A4 (SDA) and A5 (SCL) on Arduino UNO / UNO Mini

// -------------------- Configuration ----------------------
// Most IR obstacle sensor modules (LM393/FC-51) output LOW when an object is detected.
// Change to HIGH if your sensor module is active-HIGH.
const int SENSOR_DETECTED = LOW;

// SG90 positional servo range is 0..180 degrees.
// Mount the sign board at the 90-degree midpoint so +90 turns to 180 and -90 turns to 0.
const int SIGN_CENTER_ANGLE = 90;   // Normal state (neutral)
const int SIGN_SIDE1_ANGLE  = 180;  // +90 deg from center (Side 1)
const int SIGN_SIDE2_ANGLE  = 0;    // -90 deg from center (Side 2)

// Hold the warning for 3 seconds after the vehicle clears the sensor.
const unsigned long ALERT_HOLD_MS    = 3000;
const unsigned long BEEP_INTERVAL_MS = 250;  // Buzzer beep toggle speed

// -------------------- State Types & Variables ------------
enum AlertSide {
  SIDE_NONE = 0,
  SIDE_1    = 1,
  SIDE_2    = 2
};

Servo warningServo;
AlertSide activeSide         = SIDE_NONE;
bool buzzerOn                = false;
unsigned long lastDetectTime = 0;
unsigned long lastBeepTime   = 0;

void showOledSplash() {
  if (!oledReady) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(16, 12);
  display.print(F("POTHOLE WARNING"));
  display.setCursor(31, 26);
  display.print(F("ROAD SYSTEM"));
  display.setCursor(22, 46);
  display.print(F("Initializing..."));
  display.display();
}

void showOledNormal() {
  if (!oledReady) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(7, 2);
  display.print(F("ROAD STATUS: NORMAL"));
  display.drawLine(0, 13, 127, 13, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(10, 22);
  display.print(F("ALL CLEAR"));

  display.setTextSize(1);
  display.setCursor(28, 48);
  display.print(F("Drive Safely"));
  display.display();
}

void showOledAlert(AlertSide side) {
  if (!oledReady) return;
  display.clearDisplay();

  // Inverted warning header banner
  display.fillRect(0, 0, 128, 18, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(2);
  display.setCursor(16, 1);
  display.print(F("WARNING!"));

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(22, 22);
  display.print(F("POTHOLE AHEAD!"));

  display.setTextSize(2);
  display.setCursor(10, 34);
  display.print(F("SLOW DOWN"));

  display.setTextSize(1);
  display.setCursor(13, 55);
  if (side == SIDE_1) {
    display.print(F("Car: Side 1 (+90)"));
  } else {
    display.print(F("Car: Side 2 (-90)"));
  }
  display.display();
}

void setNormalState() {
  activeSide = SIDE_NONE;
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  buzzerOn = false;
  warningServo.write(SIGN_CENTER_ANGLE);
  showOledNormal();
}

void activateAlertForSide(AlertSide side, unsigned long now) {
  bool wasInactive = (activeSide == SIDE_NONE);
  activeSide = side;

  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
  warningServo.write(side == SIDE_1 ? SIGN_SIDE1_ANGLE : SIGN_SIDE2_ANGLE);

  if (wasInactive) {
    buzzerOn = true;
    digitalWrite(BUZZER_PIN, HIGH);
    lastBeepTime = now;
  }

  showOledAlert(side);
}

void setup() {
  pinMode(IR_SENSOR_1_PIN, INPUT);
  pinMode(IR_SENSOR_2_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  warningServo.attach(SERVO_PIN);
  Serial.begin(9600);

  oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR);
  if (!oledReady) {
    Serial.println(F("WARNING: GME12864-4 OLED not found at 0x3C. Check A4/A5 wiring."));
  } else {
    showOledSplash();
    delay(1000);
  }

  setNormalState();
  Serial.println(F("Intelligent Pothole Warning Road (Dual IR) initialized."));
}

void loop() {
  unsigned long now = millis();
  bool side1Detected = (digitalRead(IR_SENSOR_1_PIN) == SENSOR_DETECTED);
  bool side2Detected = (digitalRead(IR_SENSOR_2_PIN) == SENSOR_DETECTED);
  bool anyDetected   = (side1Detected || side2Detected);

  if (anyDetected) {
    lastDetectTime = now;

    // Determine which side triggered (if both trigger simultaneously, keep activeSide or default to SIDE_1)
    AlertSide targetSide = activeSide;
    if (side1Detected && !side2Detected) {
      targetSide = SIDE_1;
    } else if (side2Detected && !side1Detected) {
      targetSide = SIDE_2;
    } else if (targetSide == SIDE_NONE) {
      targetSide = SIDE_1;
    }

    if (targetSide != activeSide) {
      activateAlertForSide(targetSide, now);
      if (targetSide == SIDE_1) {
        Serial.println(F("ALERT: Side 1 vehicle detected! Servo +90 (180 deg)."));
      } else {
        Serial.println(F("ALERT: Side 2 vehicle detected! Servo -90 (0 deg)."));
      }
    }
  }

  if (activeSide != SIDE_NONE) {
    // Keep pulsing the buzzer while warning is active
    if (now - lastBeepTime >= BEEP_INTERVAL_MS) {
      lastBeepTime = now;
      buzzerOn = !buzzerOn;
      digitalWrite(BUZZER_PIN, buzzerOn ? HIGH : LOW);
    }

    // Return to normal state once both sensors are clear for ALERT_HOLD_MS
    if (!anyDetected && (now - lastDetectTime >= ALERT_HOLD_MS)) {
      setNormalState();
      Serial.println(F("NORMAL: Road clear. Servo centered (90 deg), Green LED ON."));
    }
  }

  delay(20); // Short debounce/stability delay
}
