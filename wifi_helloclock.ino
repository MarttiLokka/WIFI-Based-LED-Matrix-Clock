// Wifi clock based on Hello World and Font examples from MajicDesigns
// libraries with some vibe support from chatgpt.
//
// Font editor at https://pjrp.github.io/MDParolaFontEditor
// Automatic timezone + DST handling
//
// Pico 2W
// CLK (SCK) = GPIO18
// DATA (MOSI) = GPIO19
// CS = GPIO17
//

#include <WiFi.h>
#include <time.h>

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "Font_Data.h"

// =========================
// WIFI
// =========================
struct WiFiNetwork {
  const char* ssid;
  const char* password;
};

// Known networks
// Do not publish real passwords in a public GitHub repository.
WiFiNetwork networks[] = {
  {"NETWORK1", "PASSWORD1"},
  {"NETWORK2", "PASSWORD2"},
  {"NETWORK3", "PASSWORD3"}
};

const int networkCount =
  sizeof(networks) / sizeof(networks[0]);

// =========================
// TIMEZONE
// =========================
//
// Example:
// Finland:
// EET  = UTC+2
// EEST = UTC+3 (DST)
//
const char* LOCAL_TIMEZONE =
  "EET-2EEST,M3.5.0/3,M10.5.0/4";

// =========================
// LED MATRIX
// =========================

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define CS_PIN   17

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// =========================
// TIMING
// =========================
unsigned long previousMillis = 0;
const unsigned long interval = 500;

bool toggle = false;

// WiFi reconnect timing
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 10000;

// =========================
// WIFI CONNECT
// =========================
bool connectWiFi() {

  Serial.println("Scanning WiFi networks...");

  int n = WiFi.scanNetworks();

  // No WiFi networks at all.
  if (n == 0) {
    Serial.println("No WiFi networks found.");
    Serial.println("No conditions to continue. Halting.");
    return false;
  }

  Serial.print("Found ");
  Serial.print(n);
  Serial.println(" networks");

  bool knownNetworkFound = false;

  for (int i = 0; i < n; i++) {

    String foundSSID = WiFi.SSID(i);

    Serial.print("Found: ");
    Serial.println(foundSSID);

    for (int j = 0; j < networkCount; j++) {

      if (foundSSID == networks[j].ssid) {

        knownNetworkFound = true;

        Serial.print("Connecting to: ");
        Serial.println(networks[j].ssid);

        WiFi.begin(
          networks[j].ssid,
          networks[j].password
        );

        int attempts = 0;

        while (
          WiFi.status() != WL_CONNECTED &&
          attempts < 20
        ) {
          delay(500);
          Serial.print(".");
          attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {

          Serial.println("\nConnected!");

          Serial.print("SSID: ");
          Serial.println(networks[j].ssid);

          Serial.print("IP: ");
          Serial.println(WiFi.localIP());

          return true;
        }

        Serial.println(
          "\nConnection failed."
        );
      }
    }
  }

  // WiFi networks exist, but none of them are in the saved list.
  if (!knownNetworkFound) {
    Serial.println("No known WiFi networks found.");
    Serial.println("No conditions to continue. Halting.");
    return false;
  }

  // A known network was found, but connection failed.
  Serial.println("Could not connect to any known WiFi network.");
  Serial.println("No conditions to continue. Halting.");
  return false;
}

// =========================
// GET LOCAL TIME
// =========================
void getLocalTimeHM(
  int &hour,
  int &minute
) {

  time_t now = time(nullptr);

  struct tm timeinfo;

  localtime_r(&now, &timeinfo);

  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
}

// =========================
// SETUP
// =========================
void setup() {

  Serial.begin(115200);

  if (!connectWiFi()) {
    // Keep the display completely black and stop here.
    while (true) {
      delay(1000);
    }
  }

  // Configure NTP
  configTime(
    0,
    0,
    "pool.ntp.org",
    "time.nist.gov"
  );

  // Set timezone + DST rules
  setenv("TZ", LOCAL_TIMEZONE, 1);
  tzset();

  // Wait until time is received
  Serial.print("Getting time");

  time_t now = time(nullptr);

  while (now < 100000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println("\nTime received!");

  // LED matrix setup
  P.begin();

  P.setFont(digit);               // font name
  P.setCharSpacing(1);            // character spacing in pixels
  P.setTextAlignment(PA_CENTER);  // text alignment
  P.setIntensity(0);              // brightness
}

// =========================
// LOOP
// =========================
void loop() {

  // Reconnect WiFi if disconnected
  if (WiFi.status() != WL_CONNECTED) {

    unsigned long nowMillis = millis();

    if (
      nowMillis - lastReconnectAttempt >
      reconnectInterval
    ) {

      lastReconnectAttempt = nowMillis;

      if (!connectWiFi()) {
        // Keep the display black if no known WiFi network is available.
        while (true) {
          delay(1000);
        }
      }

      // Re-sync time after reconnect
      configTime(
        0,
        0,
        "pool.ntp.org",
        "time.nist.gov"
      );
    }
  }

  unsigned long currentMillis = millis();

  if (
    currentMillis - previousMillis >=
    interval
  ) {

    previousMillis = currentMillis;

    int h, m;

    getLocalTimeHM(h, m);

    char buf[10];

    // Blink colon AKA Here comes the clock.
    if (toggle) {
      snprintf(buf, sizeof(buf), "%02d:%02d", h, m);
    } else {
      snprintf(buf, sizeof(buf), "%02d %02d", h, m);
    }

    P.print(buf);

    toggle = !toggle;
  }
}