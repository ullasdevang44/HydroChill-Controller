#include <OneWire.h>
#include <DallasTemperature.h>

// --- Pin definitions (simplified for PCB) ---
#define ONE_WIRE_BUS 3      // DS18B20 data pin (moved to D3)
#define RELAY_PIN 2         // Relay (active LOW, moved to D2)
#define L298_IN1 4          // Fan A IN1
#define L298_IN2 5          // Fan A IN2
#define L298_IN3 6          // Fan B IN3
#define L298_IN4 7          // Fan B IN4
#define HEAT_PIN 8          // Heating pad MOSFET gate
#define BUTTON_COOL_PIN 9   // Cooling override button
#define BUTTON_HEAT_PIN 10  // Heating override button

// --- DS18B20 setup ---
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(L298_IN1, OUTPUT);
  pinMode(L298_IN2, OUTPUT);
  pinMode(L298_IN3, OUTPUT);
  pinMode(L298_IN4, OUTPUT);
  pinMode(HEAT_PIN, OUTPUT);

  pinMode(BUTTON_COOL_PIN, INPUT_PULLUP);
  pinMode(BUTTON_HEAT_PIN, INPUT_PULLUP);

  // Start with everything OFF
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(L298_IN1, LOW);
  digitalWrite(L298_IN2, LOW);
  digitalWrite(L298_IN3, LOW);
  digitalWrite(L298_IN4, LOW);
  digitalWrite(HEAT_PIN, LOW);

  Serial.begin(9600);
  sensors.begin();

  Serial.println("Temp Controller Ready");
}

// --- Control functions ---
void controlFans(bool enable) {
  if (enable) {
    digitalWrite(L298_IN1, LOW);
    digitalWrite(L298_IN2, HIGH);
    digitalWrite(L298_IN3, LOW);
    digitalWrite(L298_IN4, HIGH);
    Serial.println("Fans ON");
  } else {
    digitalWrite(L298_IN1, LOW);
    digitalWrite(L298_IN2, LOW);
    digitalWrite(L298_IN3, LOW);
    digitalWrite(L298_IN4, LOW);
    Serial.println("Fans OFF");
  }
}

void controlHeater(bool enable) {
  digitalWrite(HEAT_PIN, enable ? HIGH : LOW);
  Serial.print("Heater: ");
  Serial.println(enable ? "ON" : "OFF");
}

void controlRelay(bool enable) {
  digitalWrite(RELAY_PIN, enable ? LOW : HIGH); // Active LOW relay
  Serial.print("Relay Load: ");
  Serial.println(enable ? "ON" : "OFF");
}

// --- Main loop ---
void loop() {
  bool manualCool = (digitalRead(BUTTON_COOL_PIN) == LOW);
  bool manualHeat = (digitalRead(BUTTON_HEAT_PIN) == LOW);

  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  // If both switches are pressed → LOCK state
  if (manualCool && manualHeat) {
    controlFans(false);
    controlHeater(false);
    controlRelay(false);
    Serial.println("LOCK");
  }
  else if (manualCool) {
    // Force Fans ON, Heater OFF
    controlFans(true);
    controlHeater(false);
    controlRelay(true);
    Serial.println("MANUAL COOL");
  }
  else if (manualHeat) {
    // Force Heater ON, Fans OFF
    controlFans(false);
    controlHeater(true);
    controlRelay(false);
    Serial.println("MANUAL HEAT");
  }
  else {
    // Automatic mode
    if (tempC == DEVICE_DISCONNECTED_C) {
      controlFans(false);
      controlHeater(false);
      controlRelay(false);
      Serial.println("ERROR: Sensor disconnected");
    }
    else if (tempC > 30.0) {
      controlFans(true);
      controlHeater(false);
      controlRelay(true);
      Serial.print("AUTO COOL | Temp: ");
      Serial.println(tempC);
    }
    else if (tempC < 24.0) {
      controlFans(false);
      controlHeater(true);
      controlRelay(false);
      Serial.print("AUTO HEAT | Temp: ");
      Serial.println(tempC);
    }
    else {
      controlFans(false);
      controlHeater(false);
      controlRelay(false);
      Serial.print("IDLE | Temp: ");
      Serial.println(tempC);
    }
  }

  delay(500);
}
