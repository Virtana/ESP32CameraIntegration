/* FreeRTOS includes. */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


/* AWS System includes. */
#include "iot_system_init.h"
#include "iot_logging_task.h"
#include "iot_mqtt.h"
#include "iot_config.h"

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
#include "abr_mqtt_init.h"
#include "abr_mqtt.h"

#include "stdio.h"

QueueHandle_t queue_handle;

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


int app_main(void)
{
    prvMiscInitialization();

    queue_handle = xQueueCreate(20,sizeof(long int)); //holds 10 detected tags (and timestamp for each)

    if(queue_handle == NULL)
    {
        configPRINTF(("Failed to create queue\n"));
        return -1;
    }

    printf("QUEUE DEBUG: %i\n", uxQueueMessagesWaiting(queue_handle));

    if(SYSTEM_Init() == pdPASS)
    {
        mqtt_main(&queue_handle);
    }

    initialize_camera();

    //Stack width defined by macro portSTACK_TYPE = uint8_t. Stack size = StackDepth x sizeof(portSTACK_TYPE) = StackDepth
    //StackDepth of type uint16_t so max stack size is 0xFFFF = 65535
    xTaskCreate(capture_image,"CaptureImageTask",65535,(void*)&queue_handle,4,NULL);

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
