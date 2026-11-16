/*
 * Intelligent Pothole Warning Road (Arduino Based)
 * ------------------------------------------------
 * Detects an approaching vehicle near a pothole using an IR obstacle sensor
 * and warns the driver with:
 *   - Red Warning LED (ON) & Green Normal LED (OFF)
 *   - Pulsing 5V Buzzer alarm
 *   - SG90 Servo Motor raising the "POTHOLE AHEAD - SLOW DOWN" sign
 *
 * When the road is clear, the sign lowers, the Red LED and Buzzer turn OFF,
 * and the Green LED turns ON.
 */

#include <Servo.h>

// -------------------- Pin Definitions --------------------
const int IR_SENSOR_PIN = 2;   // IR Sensor OUT pin
const int BUZZER_PIN    = 6;   // 5V Active Buzzer (+) pin
const int RED_LED_PIN   = 7;   // Red LED (+) via 220-ohm resistor
const int GREEN_LED_PIN = 8;   // Green LED (+) via 220-ohm resistor
const int SERVO_PIN     = 9;   // SG90 Servo signal (orange) wire

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
bool alertActive          = false;
bool buzzerOn             = false;
unsigned long lastDetectTime = 0;
unsigned long lastBeepTime   = 0;

void setNormalState() {
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  buzzerOn = false;
  warningServo.write(SIGN_DOWN_ANGLE);
}

void startAlertState(unsigned long now) {
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
  warningServo.write(SIGN_UP_ANGLE);

  buzzerOn = true;
  digitalWrite(BUZZER_PIN, HIGH);
  lastBeepTime = now;
}

void setup() {
  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  warningServo.attach(SERVO_PIN);
  Serial.begin(9600);

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
      Serial.println(F("ALERT: Vehicle near pothole! Sign raised, Red LED & Buzzer ON."));
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
