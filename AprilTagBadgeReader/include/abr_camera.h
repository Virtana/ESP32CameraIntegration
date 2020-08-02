#ifndef ABR_CAMERA_H
#define ABR_CAMERA_H

#include "esp_camera.h"
#include "esp_system.h"

/*
Configure camera settings and detect and initialize camera driver
*/
esp_err_t initialize_camera();


/*
Captures an image, creates a frame buffer and calls function "detect_apriltags".
*/
void capture_image(void* pvParameters);

#endif
