#include "gallery.h"

extern M5Canvas canvas;

#define GRID_COLS   2
#define GRID_ROWS   2
#define TOTAL_CARDS (GRID_COLS * GRID_ROWS)

const char* cardPathPrefix = "/card";
const char* cardPathSuffix = ".png";

void showThumbnails(int selectedIndex) {
  int cellW = M5.Display.width() / GRID_COLS;
  int cellH = M5.Display.height() / GRID_ROWS;

  M5.Display.setEpdMode(epd_mode_t::epd_quality);
  canvas.fillSprite(WHITE);

  for (int i = 0; i < TOTAL_CARDS; i++) {
    int col = i % GRID_COLS;
    int row = i / GRID_COLS;
    int x   = col * cellW;
    int y   = row * cellH;

    String cardPath = String(cardPathPrefix) + String(i + 1) + String(cardPathSuffix);
    if (SD.exists(cardPath)) {
      canvas.drawPngFile(SD, cardPath.c_str(), x, y, cellW, cellH, 0, 0, 0.5, 0.5);
    } else {
      canvas.fillRect(x, y, cellW, cellH, LIGHTGREY);
      canvas.setCursor(x + 10, y + 10);
      canvas.setTextColor(BLACK, LIGHTGREY);
      canvas.print("No Image");
    }

    if (i == selectedIndex) {
      canvas.drawRect(x + 2, y + 2, cellW - 4, cellH - 4, BLACK);
      canvas.drawRect(x + 3, y + 3, cellW - 6, cellH - 6, BLACK);
    }
  }

  canvas.pushSprite(0, 0);
}

String selectFromGallery() {
  while (true) {
    M5.update();

    auto t = M5.Touch.getDetail(0);
    if (t.wasReleased()) {
      int cellW = M5.Display.width() / GRID_COLS;
      int cellH = M5.Display.height() / GRID_ROWS;
      int col   = t.x / cellW;
      int row   = t.y / cellH;
      int idx   = row * GRID_COLS + col;

      if (idx >= 0 && idx < TOTAL_CARDS) {
        String cardPath = String(cardPathPrefix) + String(idx + 1) + String(cardPathSuffix);
        if (SD.exists(cardPath.c_str())) {
          return cardPath;
        }
      }
    }
    delay(10);
  }
}
