#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <string.h>

/* AWS System includes. */
#include "iot_system_init.h"
#include "iot_logging_task.h"
#include "iot_mqtt.h"

#include "nvs_flash.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_interface.h"

#include "aws_application_version.h"
#include "aws_clientcredential.h"
#include "aws_clientcredential_keys.h"

#include "platform/iot_clock.h"
#include "platform/iot_threads.h"
#include "iot_demo_logging.h"
#include "iot_init.h"
#include "iot_network_manager_private.h"


#define MQTT_TIMEOUT_MS 5000
#define TOPIC_FILTER_COUNT 4

#define IOT_DEMO_MQTT_TOPIC_PREFIX "badgeReader"
#define TOPIC_FILTER_LENGTH ( ( uint16_t ) ( sizeof( IOT_DEMO_MQTT_TOPIC_PREFIX "/topic/1" ) - 1 ) )
#define PUBLISH_RETRY_LIMIT ( 10 )
#define PUBLISH_RETRY_MS ( 1000 )
#define CLIENT_IDENTIFIER_MAX_LENGTH ( 24 )
#define CLIENT_IDENTIFIER_PREFIX "badgeReader"

int initialize_mqtt()
{
    IotMqttError_t mqttInitStatus = IOT_MQTT_SUCCESS;

    mqttInitStatus = IotMqtt_Init();

    if(mqttInitStatus != IOT_MQTT_SUCCESS)
    {
        configPRINTF(("Failed to initialize MQTT\n"));
        return EXIT_FAILURE;
    }

    return IOT_MQTT_SUCCESS;
}

int connect_mqtt(bool awsIotMqttMode, const char* pIdentifier, void* pNetworkServerInfo, void* pNetworkCredentialInfo, const IotNetworkInterface_t* pNetworkInterface, IotMqttConnection_t* pMqttConnection)
{
    int status = EXIT_SUCCESS;
    IotMqttError_t connectStatus = IOT_MQTT_STATUS_PENDING;
    IotMqttNetworkInfo_t networkInfo = IOT_MQTT_NETWORK_INFO_INITIALIZER;
    IotMqttConnectInfo_t connectInfo = IOT_MQTT_CONNECT_INFO_INITIALIZER;
    char pClientIdentifierBuffer[ CLIENT_IDENTIFIER_MAX_LENGTH ] = { 0 };

    /* Set the members of the network info not set by the initializer. This
     * struct provided information on the transport layer to the MQTT connection. */
    networkInfo.createNetworkConnection = true;
    networkInfo.u.setup.pNetworkServerInfo = pNetworkServerInfo;
    networkInfo.u.setup.pNetworkCredentialInfo = pNetworkCredentialInfo;
    networkInfo.pNetworkInterface = pNetworkInterface;

    #if ( IOT_MQTT_ENABLE_SERIALIZER_OVERRIDES == 1 ) && defined( IOT_DEMO_MQTT_SERIALIZER )
        networkInfo.pMqttSerializer = IOT_DEMO_MQTT_SERIALIZER;
    #endif

    /* Set the members of the connection info not set by the initializer. */
    connectInfo.awsIotMqttMode = awsIotMqttMode;
    connectInfo.cleanSession = true;
    connectInfo.keepAliveSeconds = 60;

    /* Use the parameter client identifier if provided. Otherwise, generate a
     * unique client identifier. */
    if( ( pIdentifier != NULL ) && ( pIdentifier[ 0 ] != '\0' ) )
    {
        connectInfo.pClientIdentifier = pIdentifier;
        connectInfo.clientIdentifierLength = ( uint16_t ) strlen( pIdentifier );
    }
    else
    {
        /* Every active MQTT connection must have a unique client identifier. The demos
         * generate this unique client identifier by appending a timestamp to a common
         * prefix. */
        status = snprintf( pClientIdentifierBuffer,
                           CLIENT_IDENTIFIER_MAX_LENGTH,
                           CLIENT_IDENTIFIER_PREFIX "%lu",
                           ( long unsigned int ) IotClock_GetTimeMs() );

        /* Check for errors from snprintf. */
        if( status < 0 )
        {
            IotLogError( "Failed to generate unique client identifier for demo." );
            status = EXIT_FAILURE;
        }
        else
        {
            /* Set the client identifier buffer and length. */
            connectInfo.pClientIdentifier = pClientIdentifierBuffer;
            connectInfo.clientIdentifierLength = ( uint16_t ) status;

            status = EXIT_SUCCESS;
        }
    }

    /* Establish the MQTT connection. */
    if( status == EXIT_SUCCESS )
    {
        IotLogInfo( "MQTT demo client identifier is %.*s (length %hu).",
                    connectInfo.clientIdentifierLength,
                    connectInfo.pClientIdentifier,
                    connectInfo.clientIdentifierLength );

        connectStatus = IotMqtt_Connect( &networkInfo,
                                         &connectInfo,
                                         MQTT_TIMEOUT_MS,
                                         pMqttConnection );

        if( connectStatus != IOT_MQTT_SUCCESS )
        {
            IotLogError( "MQTT CONNECT returned error %s.",
                         IotMqtt_strerror( connectStatus ) );

            status = EXIT_FAILURE;
        }
    }

    return status;
}

int modify_subscriptions(IotMqttConnection_t mqttConnection,IotMqttOperationType_t operation,const char** pTopicFilters)
{
    int status = EXIT_SUCCESS;
    IotMqttSubscription_t pSubscriptions[TOPIC_FILTER_COUNT] = {IOT_MQTT_SUBSCRIPTION_INITIALIZER};
    IotMqttError_t subscriptionStatus = IOT_MQTT_STATUS_PENDING;

    for(uint8_t i = 0; i < TOPIC_FILTER_COUNT; i++)
    {
        pSubscriptions[ i ].qos = IOT_MQTT_QOS_1;
        pSubscriptions[ i ].pTopicFilter = pTopicFilters[ i ];
        pSubscriptions[ i ].topicFilterLength = TOPIC_FILTER_LENGTH;
        //pSubscriptions[ i ].callback.pCallbackContext = pCallbackParameter;
        //pSubscriptions[ i ].callback.function = _mqttSubscriptionCallback;
    }

    if(operation == IOT_MQTT_SUBSCRIBE)
    {
        subscriptionStatus = IotMqtt_TimedSubscribe(mqttConnection,pSubscriptions,TOPIC_FILTER_COUNT,0,MQTT_TIMEOUT_MS);

        switch(subscriptionStatus)
        {
            case IOT_MQTT_SUCCESS:
                IotLogInfo( "All demo topic filter subscriptions accepted." );
                break;

            case IOT_MQTT_SERVER_REFUSED:

                /* Check which subscriptions were rejected before exiting the demo. */
                for(uint8_t i = 0; i < TOPIC_FILTER_COUNT; i++ )
                {
                    if( IotMqtt_IsSubscribed( mqttConnection,
                                              pSubscriptions[ i ].pTopicFilter,
                                              pSubscriptions[ i ].topicFilterLength,
                                              NULL ) == true )
                    {
                        IotLogInfo( "Topic filter %.*s was accepted.",
                                    pSubscriptions[ i ].topicFilterLength,
                                    pSubscriptions[ i ].pTopicFilter );
                    }
                    else
                    {
                        IotLogError( "Topic filter %.*s was rejected.",
                                     pSubscriptions[ i ].topicFilterLength,
                                     pSubscriptions[ i ].pTopicFilter );
                    }
                }

                status = EXIT_FAILURE;
                break;

            default:

                status = EXIT_FAILURE;
                break;
        }
    }else if(operation == IOT_MQTT_UNSUBSCRIBE)
    {
        subscriptionStatus = IotMqtt_TimedUnsubscribe(mqttConnection,pSubscriptions,TOPIC_FILTER_COUNT,0,MQTT_TIMEOUT_MS);

        /* Check the status of UNSUBSCRIBE. */
        if( subscriptionStatus != IOT_MQTT_SUCCESS )
        {
            status = EXIT_FAILURE;
        }
    }

    return status;
}

int publish(IotMqttConnection_t mqttConnection,const char ** pTopicNames)
{
    int status = EXIT_SUCCESS;
    IotMqttError_t publishStatus = IOT_MQTT_STATUS_PENDING;
    IotMqttPublishInfo_t publishInfo = IOT_MQTT_PUBLISH_INFO_INITIALIZER;
    IotMqttCallbackInfo_t publishComplete = IOT_MQTT_CALLBACK_INFO_INITIALIZER;
    char pPublishPayload[20] = { 0 };

    publishInfo.qos = IOT_MQTT_QOS_1;
    publishInfo.topicNameLength = TOPIC_FILTER_LENGTH;
    publishInfo.pPayload = pPublishPayload;
    publishInfo.retryMs = PUBLISH_RETRY_MS;
    publishInfo.retryLimit = PUBLISH_RETRY_LIMIT;

    publishInfo.pTopicName = pTopicNames[0]; 

    //Generate publish payload
    status = snprintf(pPublishPayload,20,"ABRTEST123%i",9);

    if( status < 0 )
    {
        configPRINTF(("Failed to generate payload\n"));
        status = EXIT_FAILURE;
        return status;
    }
    else
    {
        publishInfo.payloadLength = ( size_t ) status;
        status = EXIT_SUCCESS;
    }

    publishStatus = IotMqtt_Publish(mqttConnection,&publishInfo,0,&publishComplete,NULL);

    if(publishStatus != IOT_MQTT_STATUS_PENDING)
    {
        configPRINTF(("Failed to publish message\n"));
        return 1;
    }

    return status;
}

void mqtt_main()
{
     IotMqttConnection_t mqttConnection = IOT_MQTT_CONNECTION_INITIALIZER;

    const char* topics[TOPIC_FILTER_COUNT] = 
    {
        "badgeReader/topic/1",
        "badgeReader/topic/2",
        "badgeReader/topic/3",
        "badgeReader/topic/4"
    };

    uint32_t network = AWSIOT_NETWORK_TYPE_WIFI;
    //IotNetworkInterface_t* pNetworkInterface = pNetworkInterface = AwsIotNetworkManager_GetNetworkInterface(network);
    //pConnectionParams = AwsIotNetworkManager_GetConnectionParams(network);
    //pCredentials = AwsIotNetworkManager_GetCredentials(network),

    int status = initialize_mqtt();
    
    if(status == EXIT_FAILURE)
    {
        configPRINTF(("Failed to initialize mqtt\n"));
        return;
    }

    status = connect_mqtt(true,clientcredentialIOT_THING_NAME,AwsIotNetworkManager_GetConnectionParams(network),AwsIotNetworkManager_GetCredentials(network),AwsIotNetworkManager_GetNetworkInterface(network),&mqttConnection);

    if(status == EXIT_FAILURE)
    {
        configPRINTF(("MQTT Failed to connect\n"));
        return -1;
    }

    modify_subscriptions(mqttConnection,IOT_MQTT_SUBSCRIBE,topics);

    publish(mqttConnection,topics);

    status = modify_subscriptions(mqttConnection,IOT_MQTT_UNSUBSCRIBE,topics);

    IotMqtt_Disconnect(mqttConnection, 0);
    IotMqtt_Cleanup();
}