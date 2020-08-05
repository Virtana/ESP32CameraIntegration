# AprilTagBadgeReader

This project detects apriltags using the ESP-EYE development board and uploads information about the detections to an AWS database.\
This is Track 4 of the [ESP32 Camera Integration Project](https://docs.google.com/document/d/1B1Nw_E98su2T_MYRsv42q9W8aAoufVafAxsERkIy82M/edit#heading=h.4nbkefahv1lc).

## Flashing ESP-EYE
- Run run_cmake.sh
- `cd build`
- Ensure PSRAM is enabled for image capturing and apriltag detection, because not enough memory is available.
    - `make menuconfig`
    - Component config > ESP32-specific > enable Support for external, SPI-connected RAM
- `make flash monitor`


If PSRAM is not enabled, the following error occurs:

I (353) camera: Allocating 300 KB frame buffer in PSRAM\
E (363) camera: Allocating 300 KB frame buffer Failed\
E (373) camera: Failed to allocate frame buffer\
E (373) gpio: gpio_isr_handler_remove(396): GPIO isr service is not installed, call gpio_install_isr_service() first\
E (383) camera: Camera init failed with error 0x101


## Displaying Camera Feed on HTTP Server

- Comment `#define SKIP_DISPLAY_IMAGES` in abr_config.h
- Follow the instructions above to flash the esp-eye. Ensure that PSRAM is enabled.
- Connect to the access point, "EESSID", (no password).
- In a browser go to the address _192.168.4.1/stream_

## Debug Statements

- Debug Statements can be enabled by commenting `#define SKIP_DEBUG_PRINT` in abr_config.h

