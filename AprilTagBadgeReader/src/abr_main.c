//#include "iot_config.h"

/* FreeRTOS includes. */

#include "FreeRTOS.h"
#include "task.h"


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

#include "abr_camera.h"
#include "abr_apriltags.h"
#include "abr_pin_map.h"

//#define DISPLAY_IMAGES //comment this line to skip displaying images.

#ifdef DISPLAY_IMAGES
    #include "abr_camera_wifi.h"
    #include "abr_display_image.h"
#endif

/* Logging Task Defines. */
#define mainLOGGING_MESSAGE_QUEUE_LENGTH    ( 32 )
#define mainLOGGING_TASK_STACK_SIZE         ( configMINIMAL_STACK_SIZE * 4 )

extern void vApplicationIPInit( void );

static void prvMiscInitialization( void )
{
    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();

    if( ( ret == ESP_ERR_NVS_NO_FREE_PAGES ) || ( ret == ESP_ERR_NVS_NEW_VERSION_FOUND ) )
    {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK( ret );

    /* Create tasks that are not dependent on the WiFi being initialized. */
    xLoggingTaskInitialize( mainLOGGING_TASK_STACK_SIZE, tskIDLE_PRIORITY + 5,mainLOGGING_MESSAGE_QUEUE_LENGTH );

#if AFR_ESP_LWIP
    configPRINTF( ("Initializing lwIP TCP stack\r\n") );
    tcpip_adapter_init();
#else
    configPRINTF( ("Initializing FreeRTOS TCP stack\r\n") );
    vApplicationIPInit();
#endif
}

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
    IotMqttPublishInfo_t willInfo = IOT_MQTT_PUBLISH_INFO_INITIALIZER;
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

int app_main(void)
{
    prvMiscInitialization();

    IotMqttConnection_t mqttConnection = IOT_MQTT_CONNECTION_INITIALIZER;

    const char* topic = "badge_reader/topic/1";

    uint32_t network = AWSIOT_NETWORK_TYPE_WIFI;
    pNetworkInterface = AwsIotNetworkManager_GetNetworkInterface(network);
    pConnectionParams = AwsIotNetworkManager_GetConnectionParams(network);
    pCredentials = AwsIotNetworkManager_GetCredentials(network),

    int status = initialize_mqtt();
    
    if(status == EXIT_FAILURE)
    {
        configPRINTF(("Failed to initialize mqtt\n"));
        return -1;
    }

    status = connect_mqtt(true,clientcredentialIOT_THING_NAME,AwsIotNetworkManager_GetConnectionParams(network),AwsIotNetworkManager_GetCredentials(network),AwsIotNetworkManager_GetNetworkInterface(network),mqttConnection);

    if(status == EXIT_FAILURE)
    {
        configPRINTF(("MQTT Failed to connect\n"));
        return -1;
    }

    IotMqtt_TimedSubscribe( mqttConnection, pSubscriptions,TOPIC_FILTER_COUNT,0,MQTT_TIMEOUT_MS );



    #ifdef DISPLAY_IMAGES
        initialize_camera();
        app_wifi_main();
        display_image_initialize();
    #endif

    #ifndef DISPLAY_IMAGES

        initialize_camera();

        //Stack is 32 bits wide. For 3e6 bytes allocated, stack depth = (3e6)/(32/8) = 750000
        xTaskCreate(capture_image,"CaptureImageTask",750000,NULL,5,NULL);
    #endif

    return 0;
}

#if !AFR_ESP_LWIP

void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
    uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
    system_event_t evt;

    if( eNetworkEvent == eNetworkUp )
    {
        /* Print out the network configuration, which may have come from a DHCP
         * server. */
        FreeRTOS_GetAddressConfiguration(
            &ulIPAddress,
            &ulNetMask,
            &ulGatewayAddress,
            &ulDNSServerAddress );

        evt.event_id = SYSTEM_EVENT_STA_GOT_IP;
        evt.event_info.got_ip.ip_changed = true;
        evt.event_info.got_ip.ip_info.ip.addr = ulIPAddress;
        evt.event_info.got_ip.ip_info.netmask.addr = ulNetMask;
        evt.event_info.got_ip.ip_info.gw.addr = ulGatewayAddress;
        esp_event_send( &evt );
    }
}
#endif
