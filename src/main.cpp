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

WebServer server(80);

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
  
  M5.Display.println("Waiting for ESPTouch");
  M5.Display.println("Use ESPTouch App");
  
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
    // Save the credentials
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
      ESP.restart(); // Restart to display new image
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
  M5.begin();
  // Get wakeup reason
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  M5.Log(esp_log_level_t::ESP_LOG_INFO, "Wakeup reason: %d\n", wakeup_reason);

  // Get board info
  auto board = M5.getBoard();

  // Set EPD mode to Fastest mode
  M5.Display.setEpdMode(epd_mode_t::epd_fastest);
  // Set display parameters
  //M5.Display.setFont(&fonts::FreeMonoBold24pt7b);
  M5.Display.setFont(&fonts::FreeMonoBold12pt7b);
  M5.Display.clearDisplay(WHITE);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(BLACK);

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    M5.Display.println("LittleFS init .. NG");
    esp_restart();
  }

  // Get SPI pins
  auto mosi = M5.getPin(m5::pin_name_t::sd_spi_mosi);
  auto miso = M5.getPin(m5::pin_name_t::sd_spi_miso);
  auto sclk = M5.getPin(m5::pin_name_t::sd_spi_sclk);
  auto cs = M5.getPin(m5::pin_name_t::sd_spi_cs);

  // Initialize SD card
  SPI.begin(sclk, miso, mosi);
  if (!SD.begin(cs, SPI, 4000000)) {
    M5.Display.println("SD card init .. NG");
    //esp_restart();
  }

  // Show tap message for Wi-Fi setup
  M5.Display.println("Tap for Wi-Fi Setup");
  
  // Wait for 3 seconds and check for touch input
  unsigned long startTime = millis();
  bool touchDetected = false;
  while (millis() - startTime < 3000) {
    M5.update();
    if (M5.Touch.getCount()) {
      touchDetected = true;
      break;
    }
    delay(100);
  }

  if (touchDetected) {
    // First try to connect using saved credentials if they exist
    String saved_ssid, saved_password;
    if (loadWiFiCredentials(saved_ssid, saved_password)) {
      M5.Display.println("Connecting with saved Wi-Fi...");
      WiFi.begin(saved_ssid.c_str(), saved_password.c_str());
      
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
      }
    }

    // If saved credentials don't exist or connection failed, try ESPTouch
    if (WiFi.status() != WL_CONNECTED) {
      // Start ESPTouch Smart Config
      M5.Display.println("Starting ESPTouch...");
      
      if (startSmartConfig()) {
        M5.Display.println("Wi-Fi Connected!");
        M5.Display.println(WiFi.localIP().toString());
        delay(2000);
      } else {
        M5.Display.println("ESPTouch Failed");
        delay(2000);
        ESP.restart();
      }
    }

    // If connected (either through saved credentials or ESPTouch), setup server
    if (WiFi.status() == WL_CONNECTED) {
      // Setup MDNS and web server
      if (MDNS.begin("m5paper")) {
        MDNS.addService("http", "tcp", 80);
      }
      server.on("/", HTTP_GET, handleRoot);
      server.on("/upload", HTTP_POST, []() {}, handleUpload);
      server.begin();

      M5.Display.println("AP Mode Ready");
      M5.Display.println(WiFi.localIP().toString().c_str());
      delay(3000); // Show IP address for 3 seconds

      // Display waiting image and return to loop
      M5.Display.clearDisplay(WHITE);
      M5.Display.drawPngFile(LittleFS, "/waiting.png");
      M5.Display.display();
      return;
    }
  }
  
  // If no touch detected or ESPTouch failed, turn off WiFi
  WiFi.mode(WIFI_OFF);

  // Set EPD mode to High quality mode
  M5.Display.setEpdMode(epd_mode_t::epd_quality);
  // Try to display image from SD card first
  M5.Display.clearDisplay(WHITE);
  M5.Display.setRotation(2); // Upside down for M5PaperS3
  bool success = M5.Display.drawPngFile(SD, "/card.png");

  // If SD card image not found, try LittleFS
  if (!success) {
    success = M5.Display.drawPngFile(LittleFS, "/card.png");
  }

  // If image loading failed, display the default image stored in littlefs
  if(!success){
    M5.Display.setRotation(0);
    if(board == m5::board_t::board_M5Paper){
      M5.Display.drawPngFile(LittleFS, "/default.png");
    }else{
      M5.Display.drawPngFile(LittleFS, "/default.png");
    }
  }

  // Enter into deep sleep
  M5.Log(esp_log_level_t::ESP_LOG_INFO, "Deep sleep start\n");
  M5.Power.deepSleep();
}

void loop() {
  M5.update();  // Update button/touch state

  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
  }

  // Check for long press (2 seconds)
  static uint32_t pressStartTime = 0;
  static bool isPressing = false;

  if (M5.Touch.getCount() > 0) {
    if (!isPressing) {
      pressStartTime = millis();
      isPressing = true;
    } else if ((millis() - pressStartTime) >= 2000) {  // 2 seconds long press
      // Delete Wi-Fi credentials file and card.png
      bool wifiDeleted = LittleFS.remove(WIFI_CONFIG_FILE);
      bool cardDeleted = LittleFS.remove("/card.png");
      
      if (wifiDeleted || cardDeleted) {
        //M5.Display.clearDisplay(WHITE);
        M5.Display.setCursor(0, 0);
        M5.Display.println("Removed:");
        if (wifiDeleted) M5.Display.println("- Wi-Fi credential");
        if (cardDeleted) M5.Display.println("- Card image");
        M5.Display.display();
        delay(2000);  // Show message for 2 seconds
        ESP.restart();  // Reboot
      }
      isPressing = false;  // Reset press state
    }
  } else {
    isPressing = false;  // Reset press state when touch is released
  }

  delay(10);
}