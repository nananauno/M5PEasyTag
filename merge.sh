#!/bin/bash

BUILD_DIR=".pio/build/Paper"
OUTPUT_FILE="merged-firmware.bin"
ESPTOOL="~/.platformio/packages/tool-esptoolpy/esptool.py"

BOOTLOADER="bootloader.bin"
PARTITIONS="partitions.bin"
BOOT="~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"
LITTLEFS="littlefs.bin"
APP="firmware.bin"

MERGE_COMMAND="python3 $ESPTOOL --chip esp32 merge_bin \
-o $BUILD_DIR/$OUTPUT_FILE \
--flash_mode keep \
--flash_freq keep \
--flash_size keep \
0x0100 $BUILD_DIR/$BOOTLOADER \
0x8000 $BUILD_DIR/$PARTITIONS \
0xe000 $BOOT \
0x10000 $BUILD_DIR/$APP \
0xc90000 $BUILD_DIR/$LITTLEFS"

echo "Running merge command:"
echo "$MERGE_COMMAND"

eval "$MERGE_COMMAND"

if [ $? -eq 0 ]; then
  echo "Successfully merged firmware to $BUILD_DIR/$OUTPUT_FILE"
else
  echo "Error: Failed to merge firmware."
  exit 1
fi