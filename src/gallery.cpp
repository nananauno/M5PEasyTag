#include "gallery.h"

// Grid configuration
#define GRID_COLS  2
#define GRID_ROWS  2
#define CELL_W     (540 / GRID_COLS)   // 270
#define CELL_H     (960 / GRID_ROWS)   // 480

const char* cardPathPrefix = "/card"; // Base path for card images
const char* cardPathSuffix = ".png"; // File extension for card images

void showThumbnails() {
  M5.Display.setEpdMode(epd_mode_t::epd_quality);
  M5.Display.clearDisplay(WHITE);

  for (int i = 0; i < GRID_COLS * GRID_ROWS; i++) {
    int col = i % GRID_COLS;
    int row = i / GRID_COLS;
    int x = col * CELL_W;
    int y = row * CELL_H;

    String cardPath = String(cardPathPrefix) + String(i + 1) + String(cardPathSuffix);
    if (SD.exists(cardPath)) {
      M5.Display.drawPngFile(SD, cardPath.c_str(), x, y, CELL_W, CELL_H, 0, 0, 0.5, 0.5);
    } else {
      // Draw placeholder if image doesn't exist
      M5.Display.fillRect(x, y, CELL_W, CELL_H, LIGHTGREY);
      M5.Display.setCursor(x + 10, y + 10);
      M5.Display.setTextColor(BLACK);
      M5.Display.print("No Image");
    }
  }
}

String selectFromGallery() {
  while (true) {
    M5.update();

    auto t = M5.Touch.getDetail(0);
    if (t.wasReleased()) {
      int col = t.x / CELL_W;
      int row = t.y / CELL_H;
      int idx = row * GRID_COLS + col;
      
      if (idx >= 0 && idx < (GRID_COLS * GRID_ROWS)) {
        String cardPath = String(cardPathPrefix) + String(idx + 1) + String(cardPathSuffix);

        if (SD.exists(cardPath.c_str())) {
          return cardPath;
        }
      }
    }
    delay(10);
  }
}