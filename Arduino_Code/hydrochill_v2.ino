#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// OLED config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin assignments
#define ONE_WIRE_BUS 3
#define RELAY_PIN 2
#define L298_IN1 4
#define L298_IN2 5
#define L298_IN3 6
#define L298_IN4 7
#define HEAT_PIN 8
#define BUTTON_COOL_PIN 9
#define BUTTON_HEAT_PIN 10

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Button debounce
const unsigned long DEBOUNCE_DELAY = 50;
unsigned long lastDebounceCool = 0, lastDebounceHeat = 0;
bool lastCoolState = HIGH, lastHeatState = HIGH;
bool coolButtonPressed = false, heatButtonPressed = false;

// State enum
enum State { LOCK, MANUAL_COOL, MANUAL_HEAT, AUTO_COOL, AUTO_HEAT, IDLE, ERROR };
State currentState = IDLE;

void controlFans(bool enable) {
  digitalWrite(L298_IN1, enable ? LOW : LOW);
  digitalWrite(L298_IN2, enable ? HIGH : LOW);
  digitalWrite(L298_IN3, enable ? LOW : LOW);
  digitalWrite(L298_IN4, enable ? HIGH : LOW);
}
void controlHeater(bool enable) {
  digitalWrite(HEAT_PIN, enable ? HIGH : LOW);
}
void controlRelay(bool enable) {
  digitalWrite(RELAY_PIN, enable ? LOW : HIGH);
}
void debounceButtons() {
  bool readingCool = digitalRead(BUTTON_COOL_PIN);
  if (readingCool != lastCoolState) {
    lastDebounceCool = millis();
    lastCoolState = readingCool;
  }
  if ((millis() - lastDebounceCool) > DEBOUNCE_DELAY) {
    coolButtonPressed = (readingCool == LOW);
  }
  bool readingHeat = digitalRead(BUTTON_HEAT_PIN);
  if (readingHeat != lastHeatState) {
    lastDebounceHeat = millis();
    lastHeatState = readingHeat;
  }
  if ((millis() - lastDebounceHeat) > DEBOUNCE_DELAY) {
    heatButtonPressed = (readingHeat == LOW);
  }
}

// Utility function to center text horizontally at a given y position and size 
void displayCenteredText(String text, int y, int size) {
  int16_t x1, y1;
  uint16_t w, h;
  display.setTextSize(size);
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  int cursorX = (SCREEN_WIDTH - w) / 2;
  display.setCursor(cursorX, y);
  display.print(text);
}

void displayStatus(float temp, State state) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (state == ERROR || temp == DEVICE_DISCONNECTED_C) {
    displayCenteredText("SENSOR", 10, 3);
    displayCenteredText("ERROR", 40, 3);
  } else {
    // Show temperature large at top (size 3)
    String tempStr = String(temp, 1) + " C";
    displayCenteredText(tempStr, 5, 3);

    // Show status below temperature (size 1 for MANUAL_HEAT, size 2 for others)
    String statusStr;
    int textSize = 2;
    switch (state) {
      case MANUAL_COOL: statusStr = "MANUAL FAN ON"; break;
      case MANUAL_HEAT: 
        statusStr = "MANUAL HEAT ON"; 
        textSize = 1; 
        break;
      case LOCK: statusStr = "LOCK"; break;
      case AUTO_COOL: statusStr = "FAN ON"; break;
      case AUTO_HEAT: statusStr = "HEATER ON"; break;
      case IDLE: statusStr = "IDLE"; break;
      default: statusStr = "UNKNOWN"; break;
    }
    displayCenteredText(statusStr, 48, textSize);
  }

  display.display();
}

void setOutputs(State state, float temp) {
  switch (state) {
    case LOCK:
      controlFans(false);
      controlHeater(false);
      controlRelay(false);
      break;
    case MANUAL_COOL:
      controlFans(true);
      controlHeater(false);
      controlRelay(true);
      break;
    case MANUAL_HEAT:
      controlFans(false);
      controlHeater(true);
      controlRelay(false);
      break;
    case ERROR:
      controlFans(false);
      controlHeater(false);
      controlRelay(false);
      break;
    case AUTO_COOL:
      controlFans(true);
      controlHeater(false);
      controlRelay(true);
      break;
    case AUTO_HEAT:
      controlFans(false);
      controlHeater(true);
      controlRelay(false);
      break;
    case IDLE:
      controlFans(false);
      controlHeater(false);
      controlRelay(false);
      break;
  }
  displayStatus(temp, state);
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(L298_IN1, OUTPUT);
  pinMode(L298_IN2, OUTPUT);
  pinMode(L298_IN3, OUTPUT);
  pinMode(L298_IN4, OUTPUT);
  pinMode(HEAT_PIN, OUTPUT);
  pinMode(BUTTON_COOL_PIN, INPUT_PULLUP);
  pinMode(BUTTON_HEAT_PIN, INPUT_PULLUP);

  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(L298_IN1, LOW);
  digitalWrite(L298_IN2, LOW);
  digitalWrite(L298_IN3, LOW);
  digitalWrite(L298_IN4, LOW);
  digitalWrite(HEAT_PIN, LOW);

  Serial.begin(9600);
  sensors.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.display();
  Serial.println("Temp Controller Ready");
}

void loop() {
  debounceButtons();

  if (coolButtonPressed && heatButtonPressed) {
    currentState = LOCK;
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);
    setOutputs(currentState, tempC);
    delay(500);
    return;
  } else if (coolButtonPressed) {
    currentState = MANUAL_COOL;
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);
    setOutputs(currentState, tempC);
    delay(500);
    return;
  } else if (heatButtonPressed) {
    currentState = MANUAL_HEAT;
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);
    setOutputs(currentState, tempC);
    delay(500);
    return;
  }

  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC == DEVICE_DISCONNECTED_C) {
    currentState = ERROR;
  } else if (tempC > 28.0) {
    currentState = AUTO_COOL;
  } else if (tempC < 24.0) {
    currentState = AUTO_HEAT;
  } else {
    currentState = IDLE;
  }
  setOutputs(currentState, tempC);
  delay(500);
}