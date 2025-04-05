# M5PEasyTag
Create your name tag with M5Paper series!

## Burn firmware with ESP Web Tools
Visit [here](https://app.nanana.uno/), connect your M5PaperS3/M5Paper and press Connect to try this app. This is very easy way to run M5PEasyTag on your M5Paper devices. No need to build firmware.

[ESP Web Tools](https://esphome.github.io/esp-web-tools/)

## How to use

### Using SD Card
1. Put card.png into an SD card
2. Insert the SD card to the M5PaperS3/M5Paper
3. Reset/Turn power on

### Using Wi-Fi Upload
1. Tap the screen when prompted to setup Wi-Fi
2. If you have previously connected to Wi-Fi:
   - The device will try to connect using saved credentials
3. If no saved credentials or connection fails:
   - The device will enter ESPTouch mode
   - Use the ESPTouch app on your smartphone to send Wi-Fi credentials
4. Once connected:
   - The device will display its IP address
   - Open either:
     - The displayed IP address in your web browser, or
     - `m5paper.local` (if your device supports mDNS)
   - Use the upload form to send a new card.png
   - The device will automatically restart to display the new image

You can see additional instructions on the M5PaperS3/M5Paper by turning power on without SD card.

### Resetting Device
To delete saved Wi-Fi credentials and uploaded card image:
1. Long-press the screen for 2 seconds at the upload screen
2. The device will display what was deleted
3. The device will automatically restart

## v0a12
Add M5Paper support.
## v0a7
This version only supports M5PaperS3.
