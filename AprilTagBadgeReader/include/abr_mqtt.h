#ifndef ABR_MQTT_H
#define ABR_MQTT_H

#include "iot_network_manager_private.h"

int run_mqtt( bool awsIotMqttMode,
                 const char * pIdentifier,
                 void * pNetworkServerInfo,
                 void * pNetworkCredentialInfo,
                 const IotNetworkInterface_t * pNetworkInterface, QueueHandle_t* queue_handle );

#endif