#include "iot_config.h"
#include "iot_demo_logging.h"
#include "iot_init.h"
#include "iot_mqtt.h"
#include "iot_wifi.h"
#include "platform/iot_network.h"
#include "platform/iot_threads.h"
#include "types/iot_network_types.h"
#include "iot_network_manager_private.h"
#include "aws_clientcredential.h"
#include "aws_clientcredential_keys.h"

typedef int (* mqttFunctionPtr_t)( bool awsIotMqttMode,
                                const char * pIdentifier,
                                void * pNetworkServerInfo,
                                void * pNetworkCredentialInfo,
                                const IotNetworkInterface_t * pNetworkInterface );


typedef void (* networkConnectedCallback_t)( bool awsIotMqttMode,
                                             const char * pIdentifier,
                                             void * pNetworkServerInfo,
                                             void * pNetworkCredentialInfo,
                                             const IotNetworkInterface_t * pNetworkInterface );

typedef void (* networkDisconnectedCallback_t)( const IotNetworkInterface_t * pNetworkInteface );

void runMqtt_main( void * pArgument );

typedef struct Context
{
    uint32_t networkTypes;
    mqttFunctionPtr_t mqttFunction;
    networkConnectedCallback_t networkConnectedCallback;
    networkDisconnectedCallback_t networkDisconnectedCallback;
} Context_t;
