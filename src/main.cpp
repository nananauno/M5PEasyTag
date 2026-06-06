// Copyright (c) NANANA (@nananauno). All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <SD.h> // Must be put before M5Unified.h
#include <LittleFS.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
//#include "esp_smartconfig.h"
#if __has_include(<epdiy.h>)
#include <epdiy.h> // Necessary only for M5PaperS3
#endif
#include <SPI.h>
#include <Preferences.h>
#include "gallery.h"
#ifdef PAPERCOLOR
#include <FastLED.h>
#define LED_PIN  21
#define NUM_LEDS 2
CRGB leds[NUM_LEDS];

struct BlinkTaskParams {
  int index;
  CRGB color;
  int intervalMs;
};

static TaskHandle_t blinkTaskHandle = nullptr;
static BlinkTaskParams blinkParams;

static void blinkTask(void* arg) {
  BlinkTaskParams* p = (BlinkTaskParams*)arg;
  bool state = false;
  for (;;) {
    state = !state;
    leds[p->index] = state ? p->color : CRGB::Black;
    FastLED.show();
    vTaskDelay(pdMS_TO_TICKS(p->intervalMs));
  }
}

void ledStartBlink(int index, CRGB color, int intervalMs = 300) {
  if (blinkTaskHandle) {
    vTaskDelete(blinkTaskHandle);
    blinkTaskHandle = nullptr;
  }
  blinkParams = {index, color, intervalMs};
  xTaskCreate(blinkTask, "led_blink", 4096, &blinkParams, 1, &blinkTaskHandle);
}

void ledStopBlink(int index) {
  if (blinkTaskHandle) {
    vTaskDelete(blinkTaskHandle);
    blinkTaskHandle = nullptr;
  }
  leds[index] = CRGB::Black;
  FastLED.show();
}

void ledSet(int index, CRGB color) {
  leds[index] = color;
  FastLED.show();
}
#endif

// App mode
enum class AppMode {
  Uploading,
  Gallery
};
AppMode currentMode = AppMode::Uploading;

// NVS
Preferences prefs;

WebServer server(80);

// Canvas for off-screen rendering
M5Canvas canvas(&M5.Display);

// Wi-Fi credentials file path
const char* WIFI_CONFIG_FILE = "/wifi.txt";

// Function to load Wi-Fi credentials from LittleFS
bool loadWiFiCredentials(String& ssid, String& password) {
  if (!LittleFS.exists(WIFI_CONFIG_FILE)) {
    return false;
  }

  File file = LittleFS.open(WIFI_CONFIG_FILE, "r");
  if (!file) {
    return false;
  }

  ssid = file.readStringUntil('\n');
  password = file.readStringUntil('\n');
  file.close();

  ssid.trim();
  password.trim();
  return true;
}

// Function to save Wi-Fi credentials to LittleFS
bool saveWiFiCredentials(const String& ssid, const String& password) {
  File file = LittleFS.open(WIFI_CONFIG_FILE, "w");
  if (!file) {
    return false;
  }

  file.println(ssid);
  file.println(password);
  file.close();
  return true;
}

// Function to start ESPTouch Smart Config
bool startSmartConfig() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();

  canvas.println("Waiting for ESPTouch");
  canvas.println("Use ESPTouch App");
  canvas.pushSprite(0, 0);

  // Wait for SmartConfig packet from mobile
  int attempts = 0;
  while (!WiFi.smartConfigDone() && attempts < 100) {
    delay(500);
    attempts++;
  }

  if (!WiFi.smartConfigDone()) {
    return false;
  }

  // Wait for WiFi to connect to AP
  attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    return saveWiFiCredentials(WiFi.SSID(), WiFi.psk());
  }

  return false;
}

// Handle image upload
void handleUpload() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  HTTPUpload& upload = server.upload();
  static File file;

  if (upload.status == UPLOAD_FILE_START) {
    file = LittleFS.open("/card.png", "w");
    if (!file) {
      server.send(500, "text/plain", "Failed to open file for writing");
      return;
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (file) {
      file.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (file) {
      file.close();
      server.send(200, "text/plain", "File uploaded successfully");
      ESP.restart();
    } else {
      server.send(500, "text/plain", "Failed to upload file");
    }
  }
}

// Setup upload form
void handleRoot() {
  String html = "<form method='POST' action='/upload' enctype='multipart/form-data'>"
                "<input type='file' name='image' accept='image/png'>"
                "<input type='submit' value='Upload'>"
                "</form>";
  server.send(200, "text/html", html);
}

void setup() {
  // Initialize M5Unified
  auto cfg = M5.config();
  cfg.clear_display = false;
  M5.begin();

  // Get wakeup reason
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  // Get board info
  auto board = M5.getBoard();

#ifdef PAPERCOLOR
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(64);
  leds[0] = CRGB::Green;
  leds[1] = CRGB::Black;
  FastLED.show();
#endif

  // Set EPD mode
#ifdef PAPERCOLOR
  //M5.Display.setEpdMode(epd_mode_t::epd_quality);
  M5.Display.setEpdMode(epd_mode_t::epd_fastest);
#else
  M5.Display.setEpdMode(epd_mode_t::epd_fastest);
#endif

  // Create canvas and initialize display parameters
  canvas.createSprite(M5.Display.width(), M5.Display.height());
  canvas.setFont(&fonts::FreeMonoBold12pt7b);
  canvas.setTextSize(1);
  canvas.setTextColor(BLACK, WHITE);
  canvas.fillSprite(WHITE);
  canvas.pushSprite(0, 0);

  M5.Log(esp_log_level_t::ESP_LOG_INFO, "Wakeup reason: %d\n", wakeup_reason);

  // NVS for card data
  if (!prefs.begin("m5ptag", false)) {
    canvas.fillSprite(WHITE);
    canvas.setCursor(0, 0);
    canvas.println("NVS init .. NG");
    canvas.pushSprite(0, 0);
    delay(2000);
    esp_restart();
  }

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    canvas.fillSprite(WHITE);
    canvas.setCursor(0, 0);
    canvas.println("LittleFS init .. NG");
    canvas.pushSprite(0, 0);
    esp_restart();
  }

  // Show menu — draw all elements to canvas, then push once
  canvas.fillSprite(WHITE);
#ifdef PAPERCOLOR
  canvas.drawPngFile(LittleFS, "/menu_pc.png");
  // menu_pc.png already contains the button guide
  ledStartBlink(0, CRGB::Green);
  canvas.pushSprite(0, 0);
  M5.Display.waitDisplay();
  ledStopBlink(0);
  ledSet(0, CRGB::Green);
#else
  canvas.drawPngFile(LittleFS, "/menu.png");
  canvas.setCursor(0, 0);
  canvas.println("Select mode:");
  canvas.println("Will be displayed the card image if no touch input within 5 seconds.");
  canvas.pushSprite(0, 0);
#endif

  // Get SPI pins
  auto mosi = M5.getPin(m5::pin_name_t::sd_spi_mosi);
  auto miso = M5.getPin(m5::pin_name_t::sd_spi_miso);
  auto sclk = M5.getPin(m5::pin_name_t::sd_spi_sclk);
  auto cs   = M5.getPin(m5::pin_name_t::sd_spi_cs);

  // Initialize SD card
  SPI.begin(sclk, miso, mosi);
  if (!SD.begin(cs, SPI, 4000000)) {
    canvas.println("SD card init .. NG");
    canvas.pushSprite(0, 0);
    //esp_restart();
  }

  // Mode selection
  bool inputDetected = false;
#ifdef PAPERCOLOR
  {
    const char* cardPath = nullptr;
    while (true) {
      // Wait for button input
      while (cardPath == nullptr) {
        M5.update();
        if (M5.BtnA.wasReleased())      cardPath = "/card1.png";
        else if (M5.BtnB.wasReleased()) cardPath = "/card2.png";
        else if (M5.BtnC.wasReleased()) cardPath = "/card3.png";
        delay(10);
      }

      canvas.fillSprite(WHITE);
      bool cardOk = canvas.drawPngFile(SD, cardPath);

      if (!cardOk) {
        // Show error at bottom of screen without clearing menu image
        ledSet(0, CRGB::Green);
        ledSet(1, CRGB::Red);
        canvas.fillSprite(WHITE);
        canvas.drawPngFile(LittleFS, "/menu_pc.png");
        int lineH = canvas.fontHeight();
        int y = M5.Display.height() - lineH * 2 - 4;
        canvas.setCursor(0, y);
        canvas.println(String("Not found: ") + cardPath);
        canvas.println("Press any button to retry.");
        canvas.pushSprite(0, 0);
        M5.Display.waitDisplay();

        // Reset for retry
        ledSet(1, CRGB::Black);
        cardPath = nullptr;
        continue;
      }

      // Success — display image
      ledStartBlink(1, CRGB::Aqua);
      canvas.pushSprite(0, 0);
      M5.Display.waitDisplay();
      ledStopBlink(1);
      // Both LEDs yellow for 1 second before sleep
      leds[0] = CRGB::Yellow; leds[1] = CRGB::Yellow; FastLED.show();  // LED2 aqua → yellow here
      delay(2000);
      leds[0] = CRGB::Black; leds[1] = CRGB::Black; FastLED.show();
      M5.Power.deepSleep();
    }
  }
#else
  {
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) {
      M5.update();
      if (M5.Touch.getCount()) {
        inputDetected = true;
        auto touch = M5.Touch.getDetail(0);
        currentMode = (touch.y < M5.Display.height() / 2) ? AppMode::Uploading : AppMode::Gallery;
        break;
      }
      delay(100);
    }
  }
#endif

  if (inputDetected && currentMode == AppMode::Uploading) {
    // First try to connect using saved credentials if they exist
    String saved_ssid, saved_password;
    if (loadWiFiCredentials(saved_ssid, saved_password)) {
      canvas.fillSprite(WHITE);
      canvas.setCursor(0, 0);
      canvas.println("Connecting with saved Wi-Fi...");
      canvas.pushSprite(0, 0);
      WiFi.begin(saved_ssid.c_str(), saved_password.c_str());

      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
      }
    }

    // If saved credentials don't exist or connection failed, try ESPTouch
    if (WiFi.status() != WL_CONNECTED) {
      canvas.fillSprite(WHITE);
      canvas.setCursor(0, 0);
      canvas.println("Starting ESPTouch...");
      canvas.pushSprite(0, 0);

      if (startSmartConfig()) {
        canvas.println("Wi-Fi Connected!");
        canvas.println(WiFi.localIP().toString());
        canvas.pushSprite(0, 0);
        delay(2000);
      } else {
        canvas.println("ESPTouch Failed");
        canvas.pushSprite(0, 0);
        delay(2000);
        ESP.restart();
      }
    }

    // If connected, setup server
    if (WiFi.status() == WL_CONNECTED) {
      if (MDNS.begin("m5paper")) {
        MDNS.addService("http", "tcp", 80);
      }
      server.on("/", HTTP_GET, handleRoot);
      server.on("/upload", HTTP_POST, []() {}, handleUpload);
      server.begin();

      canvas.fillSprite(WHITE);
      canvas.setCursor(0, 0);
      canvas.println("AP Mode Ready");
      canvas.println(WiFi.localIP().toString().c_str());
      canvas.pushSprite(0, 0);
      delay(3000);

      // Display waiting image and return to loop
      canvas.fillSprite(WHITE);
      canvas.drawPngFile(LittleFS, "/waiting.png");
      canvas.pushSprite(0, 0);
      M5.Display.waitDisplay();
      return;
    }
  }

  // If no input detected or ESPTouch failed, turn off WiFi
  WiFi.mode(WIFI_OFF);

  // If gallery mode, return to loop (selectFromGallery handles drawing)
  if (currentMode == AppMode::Gallery) {
#ifndef PAPERCOLOR
    showThumbnails();
#endif
    return;
  }

  // Load card path from NVS
  String cardPath = prefs.getString("card_path", "");

  // Display card image at high quality
  M5.Display.setEpdMode(epd_mode_t::epd_quality);
  canvas.fillSprite(WHITE);
  M5.Display.setRotation(2); // Upside down for M5PaperS3
  bool success = canvas.drawPngFile(SD, cardPath.c_str());

  if (!success) {
    success = canvas.drawPngFile(LittleFS, "/card.png");
  }

  if (!success) {
    M5.Display.setRotation(0);
#ifndef PAPERCOLOR
    canvas.drawPngFile(LittleFS, "/default.png");
#endif
  }

  canvas.pushSprite(0, 0);
  M5.Display.waitDisplay();

  // Enter deep sleep
  M5.Log(esp_log_level_t::ESP_LOG_INFO, "Deep sleep start\n");
  M5.Power.deepSleep();
}

void loop() {
  M5.update();

#ifndef PAPERCOLOR
  if (currentMode == AppMode::Gallery) {
    String selectedCardPath = selectFromGallery();
    prefs.putString("card_path", selectedCardPath);
    canvas.fillSprite(WHITE);
    canvas.setCursor(0, 0);
    canvas.println("Selected: " + selectedCardPath);
    canvas.pushSprite(0, 0);
    delay(2000);
    esp_restart();
  }

  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
  }

  // Check for long press (2 seconds) to reset credentials
  static uint32_t pressStartTime = 0;
  static bool isPressing = false;

  if (M5.Touch.getCount() > 0) {
    if (!isPressing) {
      pressStartTime = millis();
      isPressing = true;
    } else if ((millis() - pressStartTime) >= 2000) {
      bool wifiDeleted = LittleFS.remove(WIFI_CONFIG_FILE);
      bool cardDeleted = LittleFS.remove("/card.png");

      if (wifiDeleted || cardDeleted) {
        canvas.setCursor(0, 0);
        canvas.println("Removed:");
        if (wifiDeleted) canvas.println("- Wi-Fi credential");
        if (cardDeleted) canvas.println("- Card image");
        canvas.pushSprite(0, 0);
        delay(2000);
        ESP.restart();
      }
      isPressing = false;
    }
  } else {
    isPressing = false;
  }
#endif

  delay(10);
}
