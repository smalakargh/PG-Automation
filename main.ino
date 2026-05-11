// Blynk Template 
#define BLYNK_TEMPLATE_ID "xxxxxxxxxx"
#define BLYNK_TEMPLATE_NAME "Advance room 112"
#define BLYNK_AUTH_TOKEN "xxxxxxxxxx"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <time.h>

// WiFi Credentials 
const char* ssid     = "xxxxxxxx";
const char* password = "xxxxxxxx";

// Relay Pins 
#define FAN_PIN 26
#define LIGHT_PIN 27

//  DHT11 
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//  IR Sensors 
#define IR1 5 // Outside sensor
#define IR2 18 // Inside sensor
#define IR_TIMEOUT 2000 // ms

//  Blynk Timer 
BlynkTimer timer;

//  Fan Timer Variables 
bool fanTimerEnabled = false;
int fanOnHour = 0, fanOnMin = 0;
int fanOffHour = 0, fanOffMin = 0;
bool timeIsSet = false;
bool fanTurnedOnByTimer = false;

//  Fan Runtime Variables 
bool fanRunning = false;
unsigned long fanStartTime = 0;
float totalRuntime = 0;

//  IR / People Count Variables 
int peopleCount = 0;
bool autoModeEnabled = false;
bool ir1Triggered = false;
bool ir2Triggered = false;
unsigned long ir1Time = 0;
unsigned long ir2Time = 0;

// FORWARD DECLARATIONS

void updateRuntimeDisplay();
void updateTimerDisplay();

// RELAY CONTROL HELPER

void setFan(bool state) {
  digitalWrite(FAN_PIN, state ? LOW : HIGH);
  Blynk.virtualWrite(V1, state ? 1 : 0);
  if (state && !fanRunning) {
    fanRunning = true;
    fanStartTime = millis();
  } else if (!state && fanRunning) {
    totalRuntime += (millis() - fanStartTime) / 60000.0;
    fanRunning = false;
    updateRuntimeDisplay();
  }
}

void setLight(bool state) {
  digitalWrite(LIGHT_PIN, state ? LOW : HIGH);
  Blynk.virtualWrite(V2, state ? 1 : 0);
}

// UPDATE TIMER DISPLAY (V6)

void updateTimerDisplay() {
  if (!timeIsSet) {
    Blynk.virtualWrite(V6, "No timer set");
    return;
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int currentTotal = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  int offTotal = fanOffHour * 60 + fanOffMin;

  if (currentTotal > offTotal) {
    Blynk.virtualWrite(V6, "Timer Done");
    return;
  }

  char displayStr[30];
  sprintf(displayStr, "ON: %02d:%02d | OFF: %02d:%02d",
          fanOnHour, fanOnMin, fanOffHour, fanOffMin);
  Blynk.virtualWrite(V6, displayStr);
}

// UPDATE RUNTIME DISPLAY (V7)

void updateRuntimeDisplay() {
  float current = fanRunning
    ? totalRuntime + (millis() - fanStartTime) / 60000.0
    : totalRuntime;

  int hrs = (int)(current / 60);
  int mins = (int)(current) % 60;

  char runtimeStr[20];
  if (hrs > 0) sprintf(runtimeStr, "%dh %dm", hrs, mins);
  else sprintf(runtimeStr, "%d min", mins);

  Blynk.virtualWrite(V7, runtimeStr);
}

// DHT11 — READ AND DISPLAY (V8, V9)

void readDHT() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temp) || isnan(humidity)) {
    Serial.println("DHT read failed!");
    return;
  }

  Blynk.virtualWrite(V8, temp);
  Blynk.virtualWrite(V9, humidity);
  Serial.printf("Temp: %.1f°C Humidity: %.1f%%\n", temp, humidity);
}

//------------------- Developed By https://github.com/smalakargh

// IR SENSOR — PEOPLE COUNTING

void checkIR() {
  bool ir1 = digitalRead(IR1) == LOW; 
  bool ir2 = digitalRead(IR2) == LOW;

  // IR1 triggered
  if (ir1 && !ir1Triggered) {
    ir1Triggered = true;
    ir1Time = millis();
    Serial.println("IR1 triggered (outside)");
  }

  // IR2 triggered
  if (ir2 && !ir2Triggered) {
    ir2Triggered = true;
    ir2Time = millis();
    Serial.println("IR2 triggered (inside)");
  }

  // Both triggered — determine direction
  if (ir1Triggered && ir2Triggered) {

    if (ir1Time < ir2Time) {
      peopleCount++;
      Serial.printf("ENTERED | Inside: %d\n", peopleCount);
    } else {
      if (peopleCount > 0) peopleCount--;
      Serial.printf("EXITED | Inside: %d\n", peopleCount);
    }

    Blynk.virtualWrite(V10, peopleCount);

    if (autoModeEnabled) {
      if (peopleCount > 0) {
        setFan(true);
        setLight(true);
        Serial.println("Auto → Fan + Light ON");
      } else {
        setFan(false);
        setLight(false);
        Serial.println("Auto → Fan + Light OFF");
      }
    }

    ir1Triggered = false;
    ir2Triggered = false;
    ir1Time = 0;
    ir2Time = 0;
  }

  if (ir1Triggered && millis() - ir1Time > IR_TIMEOUT) {
    Serial.println("IR1 timeout — reset");
    ir1Triggered = false;
  }

  if (ir2Triggered && millis() - ir2Time > IR_TIMEOUT) {
    Serial.println("IR2 timeout — reset");
    ir2Triggered = false;
  }
}

//------------------- Developed By https://github.com/smalakargh

// BLYNK CONNECTED

BLYNK_CONNECTED() {
  Serial.println("Blynk connected! Syncing...");
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
  Blynk.syncVirtual(V4);
  Blynk.syncVirtual(V5);
  Blynk.syncVirtual(V11);
  updateTimerDisplay();
  updateRuntimeDisplay();
}

// BLYNK VIRTUAL PIN HANDLERS

// V1 — Fan Manual Control
BLYNK_WRITE(V1) {
  int val = param.asInt();
  if (val == 1) {
    digitalWrite(FAN_PIN, LOW);
    if (!fanRunning) {
      fanRunning = true;
      fanStartTime = millis();
    }
    Serial.println("Fan ON (manual)");
  } else {
    digitalWrite(FAN_PIN, HIGH);
    if (fanRunning) {
      totalRuntime += (millis() - fanStartTime) / 60000.0;
      fanRunning = false;
      updateRuntimeDisplay();
    }
    Serial.println("Fan OFF (manual)");
  }
}

// V2 — Light Manual Control
BLYNK_WRITE(V2) {
  int val = param.asInt();
  digitalWrite(LIGHT_PIN, val ? LOW : HIGH);
  Serial.printf("Light: %s\n", val ? "ON" : "OFF");
}

// V3 — Fan Timer Enable/Disable
BLYNK_WRITE(V3) {
  fanTimerEnabled = param.asInt();
  if (!fanTimerEnabled) {
    timeIsSet = false;
    fanTurnedOnByTimer = false;
    Blynk.virtualWrite(V6, "Timer Disabled");
  }
  Serial.printf("Fan Timer: %s\n",
    fanTimerEnabled ? "ENABLED" : "DISABLED");
}

// V4 — Fan ON Time
BLYNK_WRITE(V4) {
  long seconds = param[0].asLong();
  fanOnHour = seconds / 3600;
  fanOnMin = (seconds % 3600) / 60;
  Serial.printf("Fan ON Time: %02d:%02d\n", fanOnHour, fanOnMin);
  if (fanOffHour != 0 || fanOffMin != 0) {
    timeIsSet = true;
    updateTimerDisplay();
  }
}

// V5 — Fan OFF Time
BLYNK_WRITE(V5) {
  long seconds = param[0].asLong();
  fanOffHour = seconds / 3600;
  fanOffMin = (seconds % 3600) / 60;
  Serial.printf("Fan OFF Time: %02d:%02d\n", fanOffHour, fanOffMin);
  if (fanOnHour != 0 || fanOnMin != 0) {
    timeIsSet = true;
    updateTimerDisplay();
  }
}

// V11 — Auto Mode Enable/Disable
BLYNK_WRITE(V11) {
  autoModeEnabled = param.asInt();
  Serial.printf("Auto Mode: %s\n",
    autoModeEnabled ? "ENABLED" : "DISABLED");
  if (!autoModeEnabled) {
    Serial.println("Manual control active");
  }
}

// FAN TIMER CHECK (every 15 seconds)

void checkFanTimer() {
  if (!fanTimerEnabled) return;

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Time sync failed!");
    return;
  }

  int h = timeinfo.tm_hour;
  int m = timeinfo.tm_min;

  Serial.printf("Time: %02d:%02d\n", h, m);

  // Fan ON
  if (h == fanOnHour && m == fanOnMin && !fanTurnedOnByTimer) {
    setFan(true);
    fanTurnedOnByTimer = true;
    Serial.println("⏰ Fan Timer → ON");
    updateTimerDisplay();
  }

  // Fan OFF
  if (h == fanOffHour && m == fanOffMin && fanTurnedOnByTimer) {
    setFan(false);
    fanTurnedOnByTimer = false;
    fanTimerEnabled = false;
    timeIsSet = false;
    Blynk.virtualWrite(V3, 0);
    Blynk.virtualWrite(V6, "Timer Done");
    Serial.println("⏰ Fan Timer → OFF");
    Serial.println("✅ Fan Timer auto disabled!");
  }
}

// PERIODIC DISPLAY UPDATE (every 30 seconds)

void periodicUpdate() {
  updateTimerDisplay();
  updateRuntimeDisplay();
}

// SETUP

void setup() {
  Serial.begin(115200);

  // Relay pins
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(FAN_PIN, HIGH);
  digitalWrite(LIGHT_PIN, HIGH);

  // IR sensor pins
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);

  // DHT11
  dht.begin();

  // Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  // NTP IST (UTC +5:30)
  configTime(5 * 3600 + 30 * 60, 0, "pool.ntp.org");
  Serial.println("Syncing time...");
  delay(2000);

  // Timer intervals
  timer.setInterval(50L, checkIR); // IR every 50ms
  timer.setInterval(2000L, readDHT); // DHT every 2 sec
  timer.setInterval(15000L, checkFanTimer); // timer every 15 sec
  timer.setInterval(30000L, periodicUpdate); // display every 30 sec

  Serial.println("System Ready!");
}

// ---------------- Developed By https://github.com/smalakargh

// LOOP

void loop() {
  Blynk.run();
  timer.run();
}