cmake_minimum_required(VERSION 3.13)

project(PickDetection)

include_directories(include)

add_executable(pickdetection src/main.c src/pickdet_camera.c src/pickdet_motion.c src/pickdet_wifi.c src/pickdet_http_display.c src/pickdet_mqtt.c)

# Tell IDF build to link against this target.
set(IDF_PROJECT_EXECUTABLE pickdetection)

# Add some extra components. IDF_EXTRA_COMPONENT_DIRS is a variable used by ESP-IDF
# to collect extra components.
get_filename_component(
    EXTRA_COMPONENT_DIRS
    "../esp32-camera" ABSOLUTE
)
list(APPEND IDF_EXTRA_COMPONENT_DIRS ${EXTRA_COMPONENT_DIRS})

include_directories(BEFORE freertos-configs)

# Add FreeRTOS as a subdirectory. AFR_BOARD tells which board to target.
set(AFR_BOARD espressif.esp32_devkitc CACHE INTERNAL "")
add_subdirectory(../amazon-freertos ./build)

# Link against the mqtt library so that we can use it. Dependencies are transitively
# linked.
target_link_libraries(pickdetection PRIVATE AFR::mqtt 
                                    PRIVATE AFR::demo_mqtt
                                    PRIVATE AFR::demo_base
                                    PRIVATE AFR::wifi)
