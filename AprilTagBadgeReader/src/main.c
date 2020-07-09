#include "FreeRTOS.h"
#include "task.h"

#include "esp_system.h"
//#include "esp_wifi.h"
//#include "esp_interface.h"
//#include "esp_bt.h"

#include <stdio.h>

void hello_world_task(void* pvParameters)
{
    for(;;)
    {
        printf("Hello World\n");
        vTaskDelay(1000);   //task goes to "blocked" state for 1s
    }

}

int app_main(void)
{

    TaskHandle_t hello_world = NULL;

    xTaskCreate(hello_world_task,"Hello World",configMINIMAL_STACK_SIZE,NULL,configMAX_PRIORITIES-1,&hello_world);

    /*
    xTaskCreate(task,name,stackSize,parameters,priority,TaskHandle_t*)

    BY default configMINIMAL_STACK_SIZE set to 768 in FreeRTOSconfig.h

    configMAX_PRIORITIES = 7... defined in FreeRTOSConfig.h 
        so configMAX_PRIORITIES -1 = Maximum possible priority ?


    Task with the highest priority value executes first.

    */

   //Start running tasks.
   vTaskStartScheduler();

    return 0;
}