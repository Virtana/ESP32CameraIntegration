#include "pickdet_launcher.h"

#define configENABLED_NETWORKS      ( AWSIOT_NETWORK_TYPE_WIFI )

static IotNetworkManagerSubscription_t subscription = IOT_NETWORK_MANAGER_SUBSCRIPTION_INITIALIZER;
static IotSemaphore_t networkSemaphore;
static uint32_t connectedNetwork = AWSIOT_NETWORK_TYPE_NONE;

static uint32_t _getConnectedNetwork( Context_t * pContext )
{
    uint32_t ret = ( AwsIotNetworkManager_GetConnectedNetworks() & pContext->networkTypes );

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

static uint32_t _waitForNetworkConnection( Context_t * pContext )
{
    IotSemaphore_Wait( &networkSemaphore );

    return _getConnectedNetwork( pContext );
}

/*-----------------------------------------------------------*/

static void _onNetworkStateChangeCallback( uint32_t network,
                                           AwsIotNetworkState_t state,
                                           Context_t * pContext )
{
    const IotNetworkInterface_t * pNetworkInterface = NULL;
    void * pConnectionParams = NULL, * pCredentials = NULL;
    uint32_t disconnectedNetworks = AWSIOT_NETWORK_TYPE_NONE;

    if( ( state == eNetworkStateEnabled ) && ( connectedNetwork == AWSIOT_NETWORK_TYPE_NONE ) )
    {
        connectedNetwork = network;
        IotSemaphore_Post( &networkSemaphore );

        /* Disable the disconnected networks to save power and reclaim any unused memory. */
        disconnectedNetworks = configENABLED_NETWORKS & ( ~connectedNetwork );

        if( disconnectedNetworks != AWSIOT_NETWORK_TYPE_NONE )
        {
            AwsIotNetworkManager_DisableNetwork( disconnectedNetworks );
        }

        if( pContext->networkConnectedCallback != NULL )
        {
            pNetworkInterface = AwsIotNetworkManager_GetNetworkInterface( network );
            pConnectionParams = AwsIotNetworkManager_GetConnectionParams( network );
            pCredentials = AwsIotNetworkManager_GetCredentials( network ),

            pContext->networkConnectedCallback( true,
                                                    clientcredentialIOT_THING_NAME,
                                                    pConnectionParams,
                                                    pCredentials,
                                                    pNetworkInterface );
        }
    }
    else if( ( ( state == eNetworkStateDisabled ) || ( state == eNetworkStateUnknown ) ) &&
             ( connectedNetwork == network ) )
    {
        if( pContext->networkDisconnectedCallback != NULL )
        {
            pNetworkInterface = AwsIotNetworkManager_GetNetworkInterface( network );
            pContext->networkDisconnectedCallback( pNetworkInterface );
        }

        /* Re-enable all the networks for the demo for reconnection. */
        disconnectedNetworks = configENABLED_NETWORKS & ( ~connectedNetwork );

        if( disconnectedNetworks != AWSIOT_NETWORK_TYPE_NONE )
        {
            AwsIotNetworkManager_EnableNetwork( disconnectedNetworks );
        }

        connectedNetwork = _getConnectedNetwork( pContext );

        if( connectedNetwork != AWSIOT_NETWORK_TYPE_NONE )
        {
            if( pContext->networkConnectedCallback != NULL )
            {
                pNetworkInterface = AwsIotNetworkManager_GetNetworkInterface( connectedNetwork );
                pConnectionParams = AwsIotNetworkManager_GetConnectionParams( connectedNetwork );
                pCredentials = AwsIotNetworkManager_GetCredentials( connectedNetwork );

                pContext->networkConnectedCallback( true,
                                                        clientcredentialIOT_THING_NAME,
                                                        pConnectionParams,
                                                        pCredentials,
                                                        pNetworkInterface );
            }
        }
    }
}

static int _initialize( Context_t * pContext )
{
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
        if( IotSemaphore_Create( &networkSemaphore, 0, 1 ) != true )
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
        if( AwsIotNetworkManager_SubscribeForStateChange( pContext->networkTypes,
                                                          _onNetworkStateChangeCallback,
                                                          pContext,
                                                          &subscription ) != pdTRUE )
        {
            IotLogError( "Failed to subscribe network state change callback." );
            status = EXIT_FAILURE;
        }
    }

    /* Initialize all the  networks configured for the device. */
    if( status == EXIT_SUCCESS )
    {
        if( AwsIotNetworkManager_EnableNetwork( configENABLED_NETWORKS ) != configENABLED_NETWORKS )
        {
            IotLogError( "Failed to initialize all the networks configured for the device." );
            status = EXIT_FAILURE;
        }
    }

    if( status == EXIT_SUCCESS )
    {
        /* Wait for network configured for the function to be initialized. */
        connectedNetwork = _getConnectedNetwork( pContext );

        if( connectedNetwork == AWSIOT_NETWORK_TYPE_NONE )
        {
            /* Network not yet initialized. Block for a network to be initialized. */
            IotLogInfo( "No networks connected for the function. Waiting for a network connection. " );
            connectedNetwork = _waitForNetworkConnection( pContext );
        }
    }

    if( status == EXIT_FAILURE )
    {
        if( semaphoreCreated == true )
        {
            IotSemaphore_Destroy( &networkSemaphore );
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
    /* Disable all the networks used by the function.*/
    AwsIotNetworkManager_DisableNetwork( configENABLED_NETWORKS );
    IotSemaphore_Destroy( &networkSemaphore );
    IotSdk_Cleanup();
}

void runMqtt_main( void * pArgument )
{
    Context_t * pContext = ( Context_t * ) pArgument;
    const IotNetworkInterface_t * pNetworkInterface = NULL;
    void * pConnectionParams = NULL, * pCredentials = NULL;
    int status;

    configPRINTF(( "---------STARTING LAUNCH FUNCTION---------\n" ));

    status = _initialize( pContext );

    if( status == EXIT_SUCCESS )
    {
        IotLogInfo( "Successfully initialized the function. Network type for the function: %d", connectedNetwork );

        pNetworkInterface = AwsIotNetworkManager_GetNetworkInterface( connectedNetwork );
        pConnectionParams = AwsIotNetworkManager_GetConnectionParams( connectedNetwork );
        pCredentials = AwsIotNetworkManager_GetCredentials( connectedNetwork );

        /* Run the mqtt function. */
        status = pContext->mqttFunction( true,
                                         clientcredentialIOT_THING_NAME,
                                         pConnectionParams,
                                         pCredentials,
                                         pNetworkInterface );

        if( status == EXIT_SUCCESS )
        {
            IotLogInfo( "Function completed successfully." );
        }
        else
        {
            IotLogError( "Error running function." );
        }

        _cleanup();
    }
    else
    {
        IotLogError( "Failed to initialize the function. Exiting..." );
    }
    IotLogInfo( "-------FUNCTION RUN FINISHED-------\n" );
}
