#include "abr_sntp.h"

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_sntp.h"

static const char *TAG = "example";

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void sntp_main()
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.\n");
    obtain_time(10);
    // update 'now' variable with current time
    time(&now);
    

    char strftime_buf[64];

    // Set timezone to Eastern Standard Time and print local time
    setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);
}

//Max time allowed for attemting to synchronize time = retries * 2 seconds
void obtain_time(int retries)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK( esp_event_loop_create_default() );
    
    initialize_sntp();

    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;

    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retries) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retries);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

void initialize_sntp()
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");


    sntp_set_time_sync_notification_cb(time_sync_notification_cb);

    sntp_init();
}
