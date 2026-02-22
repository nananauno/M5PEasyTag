# M5PEasyTag
Create your name tag with M5Paper series!

## Burn firmware with ESP Web Tools
Visit [here](https://nanana.uno/apps/), connect your M5PaperS3/M5Paper and press Connect to try this app. This is very easy way to run M5PEasyTag on your M5Paper devices. No need to build firmware.

[ESP Web Tools](https://esphome.github.io/esp-web-tools/)

## How to use

### Bootup menu
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
   - The device will display its IP address
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

### v2a0
- Support multiple card images

### v1a0
- Add Wi-Fi support with ESPTouch for uploading card image

### v0a12
- Add M5Paper support

### v0a7
- This version only supports M5PaperS3
