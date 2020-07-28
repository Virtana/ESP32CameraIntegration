# AprilTagBadgeReader

To flash ESP-EYE:

run "run_cmake.sh"\
`cd ./build`\
`make flash monitor`

PSRAM must be enabled for image capturing and apriltag detection to work.

To enable external PSRAM manually (using default sdkconfig):
- Run cmake command in run_cmake.sh\
`cmake -DVENDOR=espressif -DBOARD=esp32_wrover_kit -DCMAKE_TOOLCHAIN_FILE=../amazon-freertos/tools/cmake/toolchains/xtensa-esp32.cmake -S . -B ./build`

- `cd build`
- `make menuconfig`
- Component config > ESP32-specific > enable Support for external, SPI-connected RAM


If PSRAM is not enabled, the following error occurs:

I (353) camera: Allocating 300 KB frame buffer in PSRAM\
E (363) camera: Allocating 300 KB frame buffer Failed\
E (373) camera: Failed to allocate frame buffer\
E (373) gpio: gpio_isr_handler_remove(396): GPIO isr service is not installed, call gpio_install_isr_service() first\
E (383) camera: Camera init failed with error 0x101\

