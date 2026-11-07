/*
  Adjustable Study Lamp - ESP32 + Wokwi

  Behaviour:
  - A slide switch enables/disables the system.
  - The LDR detects ambient light.
  - In low light, the white lamp LED turns on.
  - The potentiometer sets the lamp brightness using PWM.
  - The green LED indicates that the system is enabled.
  - The red LED indicates that the room is bright enough.

  The LDR module is powered from 3.3V so its analog output stays
  within the ESP32's safe input-voltage range.
*/

#include <Arduino.h>

// Analog inputs (ADC1 pins on the classic ESP32)
const int LDR_PIN = 34;
const int POT_PIN = 35;

// Digital input
const int SWITCH_PIN = 27;

// LED outputs
const int LAMP_LED_PIN = 18;   // PWM brightness control
const int GREEN_LED_PIN = 26;  // System enabled indicator
const int RED_LED_PIN = 25;    // Bright-room indicator

// LDR module AO becomes higher in darker conditions.
// Two thresholds create hysteresis and help prevent rapid flickering.
const int DARK_ON_THRESHOLD = 1800;
const int BRIGHT_OFF_THRESHOLD = 1500;

bool darkDetected = false;
unsigned long lastLogTime = 0;

void setup() {
  Serial.begin(115200);

  analogReadResolution(12);  // ESP32 ADC reading: 0 to 4095

  pinMode(SWITCH_PIN, INPUT);  // External 10k-ohm pull-down resistor
  pinMode(LAMP_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  analogWrite(LAMP_LED_PIN, 0);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  Serial.println("Adjustable Study Lamp started");
  Serial.println("Move the LDR light level and potentiometer to test it.");
}

void loop() {
  const int ldrValue = analogRead(LDR_PIN);
  const int potValue = analogRead(POT_PIN);
  const bool systemEnabled = (digitalRead(SWITCH_PIN) == HIGH);

  // Apply hysteresis: enter dark mode at one threshold and leave it at another.
  if (ldrValue > DARK_ON_THRESHOLD) {
    darkDetected = true;
  } else if (ldrValue < BRIGHT_OFF_THRESHOLD) {
    darkDetected = false;
  }

  // Convert the potentiometer reading (0-4095) to an 8-bit PWM value.
  const int brightness = map(potValue, 0, 4095, 0, 255);
  const bool lampShouldBeOn = systemEnabled && darkDetected;

  analogWrite(LAMP_LED_PIN, lampShouldBeOn ? brightness : 0);
  digitalWrite(GREEN_LED_PIN, systemEnabled ? HIGH : LOW);
  digitalWrite(RED_LED_PIN, (systemEnabled && !darkDetected) ? HIGH : LOW);

  // Print useful readings twice per second for calibration and demonstration.
  if (millis() - lastLogTime >= 500) {
    lastLogTime = millis();

    Serial.print("LDR: ");
    Serial.print(ldrValue);
    Serial.print(" | Pot: ");
    Serial.print(potValue);
    Serial.print(" | Brightness: ");
    Serial.print(brightness);
    Serial.print("/255 | Switch: ");
    Serial.print(systemEnabled ? "ON" : "OFF");
    Serial.print(" | Room: ");
    Serial.print(darkDetected ? "DARK" : "BRIGHT");
    Serial.print(" | Lamp: ");
    Serial.println((lampShouldBeOn && brightness > 0) ? "ON" : "OFF");
  }

  delay(50);
}
