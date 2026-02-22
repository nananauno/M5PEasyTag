#pragma once
#include <Arduino.h>
#include <SD.h> // Must be put before M5Unified.h
#include <M5Unified.h>

void showThumbnails();
String selectFromGallery();