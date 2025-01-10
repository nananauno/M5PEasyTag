// Copyright (c) NANANA (@nananauno). All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <SD.h> // Must be put before M5Unified.h
#include <LittleFS.h>
#include <M5Unified.h>
#if __has_include(<epdiy.h>)
#include <epdiy.h> // Necessary only for M5PaperS3
#endif
#include <SPI.h>

void setup() {
  // Initialize M5Unified
  M5.begin();
  // Get wakeup reason
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  M5.Log(esp_log_level_t::ESP_LOG_INFO, "Wakeup reason: %d\n", wakeup_reason);

  // Get board info
  auto board = M5.getBoard();

  // Set EPD mode to High quality mode
  M5.Display.setEpdMode(epd_mode_t::epd_quality);
  // Set display parameters
  M5.Display.setFont(&fonts::FreeMonoBold24pt7b);
  M5.Display.clearDisplay(WHITE);
  M5.Display.setCursor(0, 0);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(BLACK);

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    M5.Display.println("LittleFS init .. NG");
    esp_restart();
  }

  // Display loading icon
  M5.Display.drawPngFile(LittleFS, "/loading.png");

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
  
  // Try to display an image stored in SD card
  M5.Display.clearDisplay(WHITE);
  M5.Display.setRotation(2); // Upside down for M5PaperS3
  bool success = M5.Display.drawPngFile(SD, "/card.png");

  // If image loading failed, display the default image stored in littlefs
  if(!success){
    M5.Display.setRotation(0);
    if(board == m5::board_t::board_M5Paper){
      M5.Display.drawPngFile(LittleFS, "/default.png");
    }else{
      M5.Display.drawPngFile(LittleFS, "/default_s3.png");
    }
  }

  // Enter into deep sleep
  M5.Log(esp_log_level_t::ESP_LOG_INFO, "Deep sleep start\n");
  esp_deep_sleep_start();
}

void loop() {
  // nop
}