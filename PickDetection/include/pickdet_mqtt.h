#include "platform/iot_clock.h"
#include "platform/iot_threads.h"
#include "queue.h"
#include "iot_mqtt.h"
#include "pickdet_timestamp.h"

#define FORMAT_JSON "{ \"event\": \"motion detected!\", \"timestamp\": \"%ld\" }" 

struct mqttMessageVal 
{
   long int timestamp;
}message;

QueueHandle_t xStructQueue;

int pickdet_mqtt_main(bool awsIotMqttMode,
                      const char *pIdentifier,
                      void *pNetworkServerInfo,
                      void *pNetworkCredentialInfo,
                      const IotNetworkInterface_t *pNetworkInterface);
