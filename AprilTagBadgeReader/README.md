# AprilTagBadgeReader

To flash ESP-EYE:

run "run_cmake.sh"
`cd ./build`\
`make flash monitor`

run_cmake.sh generates the build files and copies a custom sdkconfig into the build directory to enable the use of external PSRAM.

To enable external PSRAM manually (using default sdkconfig):
- Run cmake command in run_cmake.sh\
`cmake -DVENDOR=espressif -DBOARD=esp32_wrover_kit -DCMAKE_TOOLCHAIN_FILE=../amazon-freertos/tools/cmake/toolchains/xtensa-esp32.cmake -S . -B ./build`

- `cd build`
- `make menuconfig`
- Component config > ESP32-specific > enable Support for external, SPI-connected RAM




