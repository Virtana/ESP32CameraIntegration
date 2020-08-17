# AprilTagBadgeReader

This project detects apriltags using the ESP-EYE development board and uploads information about the detections to an AWS database.\
This is Track 4 of the [ESP32 Camera Integration Project](https://docs.google.com/document/d/1B1Nw_E98su2T_MYRsv42q9W8aAoufVafAxsERkIy82M/edit#heading=h.4nbkefahv1lc).

To flash ESP-EYE:

run "run_cmake.sh"\
`cd ./build`\
`make flash monitor`

run_cmake.sh generates the build files and copies a custom sdkconfig into the build directory to enable the use of external PSRAM.

To enable external PSRAM manually (using default sdkconfig):
- Run cmake command in run_cmake.sh\
`cmake -DVENDOR=espressif -DBOARD=esp32_wrover_kit -DCMAKE_TOOLCHAIN_FILE=../amazon-freertos/tools/cmake/toolchains/xtensa-esp32.cmake -S . -B ./build`

To flash ESP-EYE:
- Run run_cmake.sh
- `cd build`
- Ensure PSRAM is enabled for image capturing and apriltag detection, because not enough memory is available.
    - `make menuconfig`
    - Component config > ESP32-specific > enable Support for external, SPI-connected RAM
- `make flash monitor`



I (353) camera: Allocating 300 KB frame buffer in PSRAM\
E (363) camera: Allocating 300 KB frame buffer Failed\
E (373) camera: Failed to allocate frame buffer\
E (373) gpio: gpio_isr_handler_remove(396): GPIO isr service is not installed, call gpio_install_isr_service() first\
E (383) camera: Camera init failed with error 0x101

