/*
 * Intelligent Pothole Warning Road (Arduino Based)
 * ------------------------------------------------
 * Detects an approaching vehicle near a pothole using an IR obstacle sensor
 * and warns the driver with:
 *   - Red Warning LED (ON) & Green Normal LED (OFF)
 *   - Pulsing 5V Buzzer alarm
 *   - SG90 Servo Motor raising the "POTHOLE AHEAD - SLOW DOWN" sign
 *   - GME12864-4 (SSD1306 128x64 I2C) OLED Display showing live road status
 *
 * When the road is clear, the sign lowers, the Red LED and Buzzer turn OFF,
 * the Green LED turns ON, and the OLED shows "ALL CLEAR / Drive Safely".
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
const int IR_SENSOR_PIN = 2;   // IR Sensor OUT pin
const int BUZZER_PIN    = 6;   // 5V Active Buzzer (+) pin
const int RED_LED_PIN   = 7;   // Red LED (+) via 220-ohm resistor
const int GREEN_LED_PIN = 8;   // Green LED (+) via 220-ohm resistor
const int SERVO_PIN     = 9;   // SG90 Servo signal (orange) wire
// Note: GME12864-4 OLED uses I2C pins A4 (SDA) and A5 (SCL) on Arduino UNO / UNO Mini

// -------------------- Configuration ----------------------
// Most IR obstacle sensor modules (LM393/FC-51) output LOW when an object is detected.
// Change to HIGH if your sensor module is active-HIGH.
const int SENSOR_DETECTED = LOW;

// Adjust servo angles (0-180) to match how your warning sign is mounted.
const int SIGN_DOWN_ANGLE = 0;   // Normal state (sign lowered)
const int SIGN_UP_ANGLE   = 90;  // Warning state (sign raised)

// Hold the warning for 3 seconds after the vehicle clears the sensor.
const unsigned long ALERT_HOLD_MS    = 3000;
const unsigned long BEEP_INTERVAL_MS = 250;  // Buzzer beep toggle speed

// -------------------- State Variables --------------------
Servo warningServo;
bool alertActive             = false;
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

void showOledAlert() {
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
  display.setCursor(16, 55);
  display.print(F("Vehicle Detected"));
  display.display();
}

void setNormalState() {
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  buzzerOn = false;
  warningServo.write(SIGN_DOWN_ANGLE);
  showOledNormal();
}

void startAlertState(unsigned long now) {
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
  warningServo.write(SIGN_UP_ANGLE);

  buzzerOn = true;
  digitalWrite(BUZZER_PIN, HIGH);
  lastBeepTime = now;
  showOledAlert();
}

void setup() {
  pinMode(IR_SENSOR_PIN, INPUT);
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
  Serial.println(F("Intelligent Pothole Warning Road initialized."));
}

void loop() {
  unsigned long now = millis();
  bool vehicleDetected = (digitalRead(IR_SENSOR_PIN) == SENSOR_DETECTED);

  if (vehicleDetected) {
    lastDetectTime = now;
    if (!alertActive) {
      alertActive = true;
      startAlertState(now);
      Serial.println(F("ALERT: Vehicle near pothole! Sign raised, Red LED, Buzzer & OLED ON."));
    }
  }

  if (alertActive) {
    // Keep pulsing the buzzer while warning is active
    if (now - lastBeepTime >= BEEP_INTERVAL_MS) {
      lastBeepTime = now;
      buzzerOn = !buzzerOn;
      digitalWrite(BUZZER_PIN, buzzerOn ? HIGH : LOW);
    }

    // Return to normal state once vehicle has cleared for ALERT_HOLD_MS
    if (!vehicleDetected && (now - lastDetectTime >= ALERT_HOLD_MS)) {
      alertActive = false;
      setNormalState();
      Serial.println(F("NORMAL: Road clear. Sign lowered, Green LED ON."));
    }
  }

  delay(20); // Short debounce/stability delay
}
