#include "iot_config.h"

/* FreeRTOS includes. */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Demo includes */
#include "aws_demo.h"
#include "aws_dev_mode_key_provisioning.h"

/* AWS System includes. */
#include "bt_hal_manager.h"
#include "iot_system_init.h"
#include "iot_logging_task.h"

#include "nvs_flash.h"
#if !AFR_ESP_LWIP
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#endif

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_interface.h"
#if CONFIG_NIMBLE_ENABLED == 1
    #include "esp_nimble_hci.h"
#endif

#include "driver/uart.h"
#include "aws_application_version.h"
#include "tcpip_adapter.h"

#include "iot_network_manager_private.h"

#include "iot_mqtt.h"
#include "platform/iot_clock.h"
#include "platform/iot_threads.h"
#include "iot_init.h"
#include "iot_demo_logging.h"

#include "abr_mqtt_init.h"
#include "abr_mqtt.h"

#include "abr_sntp.h"

static uint32_t demoConnectedNetwork = AWSIOT_NETWORK_TYPE_NONE;
static IotSemaphore_t demoNetworkSemaphore;
static IotNetworkManagerSubscription_t subscription = IOT_NETWORK_MANAGER_SUBSCRIPTION_INITIALIZER;

static void _onNetworkStateChangeCallback( uint32_t network,
                                           AwsIotNetworkState_t state,
                                           void * pContext );

static uint32_t _getConnectedNetworkForDemo( demoContext_t * pDemoContext )
{
    uint32_t ret = ( AwsIotNetworkManager_GetConnectedNetworks() & pDemoContext->networkTypes );

    if( ( ret & AWSIOT_NETWORK_TYPE_WIFI ) == AWSIOT_NETWORK_TYPE_WIFI )
    {
        ret = AWSIOT_NETWORK_TYPE_WIFI;
    }
    else if( ( ret & AWSIOT_NETWORK_TYPE_BLE ) == AWSIOT_NETWORK_TYPE_BLE )
    {
        ret = AWSIOT_NETWORK_TYPE_BLE;
    }
    else if( ( ret & AWSIOT_NETWORK_TYPE_ETH ) == AWSIOT_NETWORK_TYPE_ETH )
    {
        ret = AWSIOT_NETWORK_TYPE_ETH;
    }
    else
    {
        ret = AWSIOT_NETWORK_TYPE_NONE;
    }

    return ret;
}



static uint32_t _waitForDemoNetworkConnection( demoContext_t * pDemoContext )
{
    IotSemaphore_Wait( &demoNetworkSemaphore );

    return _getConnectedNetworkForDemo( pDemoContext );
}

static int _initialize()
{
    static demoContext_t mqttContext =
    {
        .networkTypes                = AWSIOT_NETWORK_TYPE_WIFI,
        .demoFunction                = NULL,
        .networkConnectedCallback    = NULL,
        .networkDisconnectedCallback = NULL
    };

    int status = EXIT_SUCCESS;
    bool commonLibrariesInitialized = false;
    bool semaphoreCreated = false;

    /* Initialize common libraries required by network manager and demo. */
    if( IotSdk_Init() == true )
    {
        commonLibrariesInitialized = true;
    }
    else
    {
        IotLogInfo( "Failed to initialize the common library." );
        status = EXIT_FAILURE;
    }

    if( status == EXIT_SUCCESS )
    {
        if( AwsIotNetworkManager_Init() != pdTRUE )
        {
            IotLogError( "Failed to initialize network manager library." );
            status = EXIT_FAILURE;
        }
    }

    if( status == EXIT_SUCCESS )
    {
        /* Create semaphore to signal that a network is available for the demo. */
        if( IotSemaphore_Create( &demoNetworkSemaphore, 0, 1 ) != true )
        {
            IotLogError( "Failed to create semaphore to wait for a network connection." );
            status = EXIT_FAILURE;
        }
        else
        {
            semaphoreCreated = true;
        }
    }

    if( status == EXIT_SUCCESS )
    {
        /* Subscribe for network state change from Network Manager. */
        if( AwsIotNetworkManager_SubscribeForStateChange( (&mqttContext)->networkTypes,
                                                          _onNetworkStateChangeCallback,
                                                          &mqttContext,
                                                          &subscription ) != pdTRUE )
        {
            IotLogError( "Failed to subscribe network state change callback." );
            status = EXIT_FAILURE;
        }
    }

    /* Initialize all the  networks configured for the device. */
    if( status == EXIT_SUCCESS )
    {
        configPRINTF(("Connecting to WiFi\n"));

        //This function calls WIFI_On() followed by WIFI_ConnectAP() using the credientials specified in aws configure.
        if( AwsIotNetworkManager_EnableNetwork( configENABLED_NETWORKS ) != configENABLED_NETWORKS )
        {
            IotLogError( "Failed to initialize all the networks configured for the device." );
            status = EXIT_FAILURE;
        }
    }

    if( status == EXIT_SUCCESS )
    {
        /* Wait for network configured for the demo to be initialized. */
        demoConnectedNetwork = _getConnectedNetworkForDemo( &mqttContext );

        if( demoConnectedNetwork == AWSIOT_NETWORK_TYPE_NONE )
        {
            /* Network not yet initialized. Block for a network to be initialized. */
            IotLogInfo( "No networks connected for the demo. Waiting for a network connection. " );
            demoConnectedNetwork = _waitForDemoNetworkConnection( &mqttContext );
        }
    }

    if( status == EXIT_FAILURE )
    {
        if( semaphoreCreated == true )
        {
            IotSemaphore_Destroy( &demoNetworkSemaphore );
        }

        if( commonLibrariesInitialized == true )
        {
            IotSdk_Cleanup();
        }
    }

    return status;
}

static void _cleanup( void )
{
    /* Remove network manager subscription */
    AwsIotNetworkManager_RemoveSubscription( subscription );
    /* Disable all the networks used by the demo.*/
    AwsIotNetworkManager_DisableNetwork( configENABLED_NETWORKS );
    IotSemaphore_Destroy( &demoNetworkSemaphore );
    IotSdk_Cleanup();
}

//This function is run in a new task. It is created in mqtt_main().
void mqtt_task(void* pvParameters)
{   
    void* pConnectionParams = NULL;
    void* pCredentials = NULL;
    int status;
    const IotNetworkInterface_t * pNetworkInterface = NULL;

    pNetworkInterface = AwsIotNetworkManager_GetNetworkInterface( demoConnectedNetwork );
    pConnectionParams = AwsIotNetworkManager_GetConnectionParams( demoConnectedNetwork );
    pCredentials = AwsIotNetworkManager_GetCredentials( demoConnectedNetwork );

    status = run_mqtt( true, clientcredentialIOT_THING_NAME, pConnectionParams,pCredentials,pNetworkInterface, (QueueHandle_t*)pvParameters);

    _cleanup();
}

void mqtt_main(QueueHandle_t* apriltag_detections_queue)
{
    vDevModeKeyProvisioning();

    demoConnectedNetwork = AWSIOT_NETWORK_TYPE_WIFI;

    _initialize();

    sntp_main();

    xTaskCreate(mqtt_task,"mqttTask",configMINIMAL_STACK_SIZE * 8,(void*)apriltag_detections_queue,4,NULL);
}

static void _onNetworkStateChangeCallback( uint32_t network,
                                           AwsIotNetworkState_t state,
                                           void * pContext )
{
    const IotNetworkInterface_t * pNetworkInterface = NULL;
    void * pConnectionParams = NULL, * pCredentials = NULL;
    uint32_t disconnectedNetworks = AWSIOT_NETWORK_TYPE_NONE;

    demoContext_t * pDemoContext = ( demoContext_t * ) pContext;

    if( ( state == eNetworkStateEnabled ) && ( demoConnectedNetwork == AWSIOT_NETWORK_TYPE_NONE ) )
    {
        demoConnectedNetwork = network;
        IotSemaphore_Post( &demoNetworkSemaphore );

        disconnectedNetworks = configENABLED_NETWORKS & ( ~demoConnectedNetwork );

        if( disconnectedNetworks != AWSIOT_NETWORK_TYPE_NONE )
        {
            AwsIotNetworkManager_DisableNetwork( disconnectedNetworks );
        }

        if( pDemoContext->networkConnectedCallback != NULL )
        {
            pNetworkInterface = AwsIotNetworkManager_GetNetworkInterface( network );
            pConnectionParams = AwsIotNetworkManager_GetConnectionParams( network );
            pCredentials = AwsIotNetworkManager_GetCredentials( network ),

            pDemoContext->networkConnectedCallback( true,
                                                    clientcredentialIOT_THING_NAME,
                                                    pConnectionParams,
                                                    pCredentials,
                                                    pNetworkInterface );
        }
    }
    else if( ( ( state == eNetworkStateDisabled ) || ( state == eNetworkStateUnknown ) ) &&
             ( demoConnectedNetwork == network ) )
    {
        if( pDemoContext->networkDisconnectedCallback != NULL )
        {
            pNetworkInterface = AwsIotNetworkManager_GetNetworkInterface( network );
            pDemoContext->networkDisconnectedCallback( pNetworkInterface );
        }

        disconnectedNetworks = configENABLED_NETWORKS & ( ~demoConnectedNetwork );

        if( disconnectedNetworks != AWSIOT_NETWORK_TYPE_NONE )
        {
            AwsIotNetworkManager_EnableNetwork( disconnectedNetworks );
        }

        demoConnectedNetwork = AWSIOT_NETWORK_TYPE_WIFI;

        if( demoConnectedNetwork != AWSIOT_NETWORK_TYPE_NONE )
        {
            if( pDemoContext->networkConnectedCallback != NULL )
            {
                pNetworkInterface = AwsIotNetworkManager_GetNetworkInterface( demoConnectedNetwork );
                pConnectionParams = AwsIotNetworkManager_GetConnectionParams( demoConnectedNetwork );
                pCredentials = AwsIotNetworkManager_GetCredentials( demoConnectedNetwork );

                pDemoContext->networkConnectedCallback( true,
                                                        clientcredentialIOT_THING_NAME,
                                                        pConnectionParams,
                                                        pCredentials,
                                                        pNetworkInterface );
            }
        }
    }
}

