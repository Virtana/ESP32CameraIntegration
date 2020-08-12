#include "pickdet_timestamp.h"

static const char *TAG = "SNTP";

long int get_time()
{
    long int now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
     if (timeinfo.tm_year < (2016 - 1900)) {
        obtain_time();
    // update 'now' variable with current time
        time(&now);
    
    }    
    return now;
    // char strftime_buf[64];
    // setenv("TZ", "<-04>4", 1);
    // tzset();
    // localtime_r(&now, &timeinfo);
    // strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    // ESP_LOGI(TAG, "The current date/time in New York is: %s", strftime_buf); 
}

void obtain_time(void)
{
    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}

void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}
