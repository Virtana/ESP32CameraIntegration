#ifndef ABR_MQTT__INIT_H
#define ABR_MQTT__INIT_H

#include "queue.h"

void mqtt_main(QueueHandle_t* apriltag_detections_queue);

#endif
