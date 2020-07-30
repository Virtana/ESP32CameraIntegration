#ifndef ABR_MQTT_H
#define ABR_MQTT_H

int run_mqtt( bool awsIotMqttMode,
                 const char * pIdentifier,
                 void * pNetworkServerInfo,
                 void * pNetworkCredentialInfo,
                 const IotNetworkInterface_t * pNetworkInterface );

#endif