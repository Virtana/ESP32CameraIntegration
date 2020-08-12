/*
 * FreeRTOS V1.4.7
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */


#include "iot_config.h"

#include "iot_system_init.h"
#include "iot_logging_task.h"

#include "nvs_flash.h"
#if !AFR_ESP_LWIP
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#endif

#include "esp_interface.h"

#include "aws_application_version.h"

#include "pickdet_camera.h"
#include "pickdet_wifi.h"
#include "pickdet_http_display.h"
#include "pickdet_launcher.h"

/* Logging Task Defines. */
#define mainLOGGING_MESSAGE_QUEUE_LENGTH    ( 32 )
#define mainLOGGING_TASK_STACK_SIZE         ( configMINIMAL_STACK_SIZE * 8 )
#define STACKSIZE   ( configMINIMAL_STACK_SIZE * 7 )

//uncomment for streaming with motion detection 
//#define HTTP_STREAM

static const char *TAG = "STATUS";

/*-----------------------------------------------------------*/

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
    xLoggingTaskInitialize( mainLOGGING_TASK_STACK_SIZE,
                            tskIDLE_PRIORITY + 5,
                            mainLOGGING_MESSAGE_QUEUE_LENGTH );

    #if AFR_ESP_LWIP
        configPRINTF( ("Initializing lwIP TCP stack\r\n") );
        tcpip_adapter_init();
    #else
        configPRINTF( ("Initializing FreeRTOS TCP stack\r\n") );
        vApplicationIPInit();
    #endif
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

/*-----------------------------------------------------------*/

int app_main( void )
{
    prvMiscInitialization();
    pickdet_cam_init();
    xStructQueue = xQueueCreate(10,sizeof(message));

    if( SYSTEM_Init() == pdPASS )
    {
        #ifdef HTTP_STREAM
            //STREAMING AND MOTION DETECTION
             pickdet_wifi_main();
             Iot_CreateDetachedThread(pickdet_http_main, NULL,tskIDLE_PRIORITY+6,3*configMINIMAL_STACK_SIZE);
        #else
            //MQTT PUBLISHING AND MOTION DETECTION - NO HTTP STREAMING
        static Context_t mqttContext =
        {
            .networkTypes                = AWSIOT_NETWORK_TYPE_WIFI,
            .mqttFunction                = pickdet_mqtt_main,
            .networkConnectedCallback    = NULL,
            .networkDisconnectedCallback = NULL
        };
        Iot_CreateDetachedThread(runMqtt_main, &mqttContext, tskIDLE_PRIORITY + 2, STACKSIZE);
        Iot_CreateDetachedThread(pickdet_motion_solo, NULL,tskIDLE_PRIORITY + 3,STACKSIZE);
        #endif
    }   
    return 0;
}
