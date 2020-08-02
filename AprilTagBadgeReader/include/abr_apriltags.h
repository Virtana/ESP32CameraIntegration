#ifndef ABR_APRILTAGS_H
#define ABR_APRILTAGS_H

#include "esp_camera.h"
#include "queue.h"

/*
Accepts a frame buffer created from the esp32-camera library. Pixel format assumed to be PIXFORMAT_GRAYSCALE.
Function copies the data buffer (fb->buf) of the image to the data buffer of an image_u8_t from the apriltag library, accounting for
different stride lengths (width+padding). 

Detects apriltags in image and prints unique ids detected.
*/
void detect_apriltags(camera_fb_t* fb,QueueHandle_t* queue_handle);

#endif
