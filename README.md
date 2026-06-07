# M5PEasyTag
Create your name tag with M5Paper series!

## Burn firmware with ESP Web Tools
Visit [here](https://nanana.uno/apps/), connect your M5PaperS3/M5Paper and press Connect to try this app. This is very easy way to run M5PEasyTag on your M5Paper devices. No need to build firmware.

[ESP Web Tools](https://esphome.github.io/esp-web-tools/)

## How to use

### M5PaperColor

#### Bootup menu
On startup, the menu image is displayed. Press one of the buttons to select a mode:

**Normal mode (no button held at boot):**
- **Button A**: Display `/card1.png` from SD card
- **Button B**: Display `/card2.png` from SD card
- **Button C**: Display `/card3.png` from SD card

The selected image is rendered and the device enters deep sleep.

**Wi-Fi upload mode (hold Button C while powering on):**  
The device connects to Wi-Fi and starts a web server. Open `m5paper.local` or the device's IP address in your browser to upload a card image.

#### Wi-Fi Upload Mode
1. Hold **Button C** while powering on
2. The device will try to connect using previously saved Wi-Fi credentials  
   (LED 1 blinks yellow while connecting)
3. If no saved credentials or connection fails:
   - The device enters ESPTouch mode (LED 1 blinks blue)
   - Use the ESPTouch app on your smartphone to send Wi-Fi credentials
   - After successful connection, credentials are saved for future use
4. Once connected, LED 1 turns solid blue
5. Open `m5paper.local` (or the IP address) in your browser
6. Upload a PNG image using the upload form
7. After a successful upload, the image is displayed and the device enters deep sleep

**While in Wi-Fi upload mode:**
- **Button A or B** (short press): Display `/card.png` (LittleFS) and enter deep sleep
- **Button C** (hold 2 seconds): Delete saved Wi-Fi credentials and restart

#### If a card file is not found
An error message is shown at the bottom of the menu screen along with the missing file path.  
Check if the file exists on the SD card.

#### LED indicators
Two NeoPixel LEDs are connected to pin 21.

| LED | State | Meaning |
|-----|-------|---------|
| LED 1 (left) | Green (solid) | Powered on — normal mode |
| LED 1 (left) | Blue (solid) | Powered on — Wi-Fi upload mode detected (Button C held) |
| LED 1 (left) | Green (blinking) | Menu image loading / EPD refreshing |
| LED 1 (left) | Green (solid) | Ready — waiting for button input |
| LED 1 (left) | Yellow (blinking) | Connecting to Wi-Fi with saved credentials |
| LED 1 (left) | Blue (blinking) | Waiting for ESPTouch |
| LED 1 (left) | Blue (solid) | Wi-Fi connected — web server running |
| LED 1 (left) | Red (solid) | Wi-Fi connection failed — restarting |
| LED 2 (right) | Cyan (blinking) | Card image loading |
| LED 2 (right) | Red (solid) | Card file not found |
| Both LEDs | Green (1 second) | Wi-Fi credentials deleted — restarting |
| Both LEDs | Yellow (2 seconds) | Image displayed successfully — entering deep sleep |
| Both LEDs | Off | Deep sleep |

---

### M5PaperS3 / M5Paper

#### Bootup menu
On startup, the device waits for 5 seconds for a touch input to determine the mode:
- **Tap the upper half**: Enter Wi-Fi upload mode
- **Tap the lower half**: Enter gallery mode
- **No tap**: Display the current card image and enter deep sleep

### Using Gallery Mode (Multiple Card Images)
1. Prepare up to 4 card images on your SD card as:
   - `/card1.png`
   - `/card2.png`
   - `/card3.png`
   - `/card4.png`
2. Insert the SD card to the M5PaperS3/M5Paper
3. Power on and tap the lower half of the screen to enter into the gallery mode
4. Select a card image
5. The device will automatically reboot and displays the selected card image

### Using Wi-Fi Upload (Upload to LittleFS)
1. Power on the device and tap the upper half of the screen
2. The device will try to connect using previously saved Wi-Fi credentials
3. If no saved credentials or connection fails:
   - The device will enter ESPTouch mode
   - Use the ESPTouch app on your smartphone to send Wi-Fi credentials
   - After successful connection, credentials are saved for future use
4. Once connected:
   - Open either:
     - The displayed IP address in your web browser, or
     - `m5paper.local` (if your device supports mDNS)
   - Use the upload form to send a new `card.png`
   - The device will automatically restart to display the new image

### Using SD Card (Simple Single Image)
1. Put `card.png` into your SD card root
2. Insert the SD card into the M5PaperS3/M5Paper
3. Power on without touching the screen (or let 5 seconds pass)
4. The device will display the image and enter deep sleep

### Resetting Device
To delete saved Wi-Fi credentials and uploaded card image:
1. Long-press the screen for 2 seconds
2. The device will display what was deleted
3. The device will automatically restart

## Version History

### v3a2
- Add Wi-Fi upload mode to M5PaperColor (hold Button C at boot)
- Upload page served from LittleFS (`/index.html`) — supports drag & drop
- After upload, card image is displayed immediately before deep sleep
- Button A/B in Wi-Fi mode displays uploaded card and enters deep sleep
- Button C long press (2s) in Wi-Fi mode resets Wi-Fi credentials

### v3a1
- Support PaperColor

### v2a0
- Support multiple card images

### v1a0
- Add Wi-Fi support with ESPTouch for uploading card image

### v0a12
- Add M5Paper support

### v0a7
- This version only supports M5PaperS3
