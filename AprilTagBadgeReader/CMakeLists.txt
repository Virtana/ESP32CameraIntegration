cmake_minimum_required(VERSION 3.13)

project(AprilTagBadgeReader)

add_executable(badgeReader src/abr_main.c src/abr_camera.c src/abr_apriltags.c src/abr_mqtt.c src/abr_mqtt_init.c src/abr_sntp.c)

# Tell IDF build to link against this target.
set(IDF_PROJECT_EXECUTABLE badgeReader)


#Add apriltag component
get_filename_component(
    EXTRA_COMPONENT_DIRS
    "components/apriltag-component" ABSOLUTE
)
list(APPEND IDF_EXTRA_COMPONENT_DIRS ${EXTRA_COMPONENT_DIRS})

get_filename_component(
    EXTRA_COMPONENT_DIRS
    "../esp32-camera" ABSOLUTE
)
list(APPEND IDF_EXTRA_COMPONENT_DIRS ${EXTRA_COMPONENT_DIRS})

# Add FreeRTOS as a subdirectory. AFR_BOARD tells which board to target.
set(AFR_BOARD espressif.esp32_devkitc CACHE INTERNAL "")

#Override FreeRTOS config ("Override the configurations for FreeRTOS" section of getting started guide)
#FreeRTOSConfig.h is added to freertos-configs to disable the use of hook functions.
include_directories(BEFORE freertos-configs)

include_directories(BEFORE ./include)

add_subdirectory(../amazon-freertos ./build)

# Link against the mqtt library so that we can use it. Dependencies are transitively
# linked.
target_link_libraries(badgeReader PRIVATE AFR::demo_mqtt)


target_link_libraries(badgeReader PRIVATE AFR::demo_ble_numeric_comparison)
