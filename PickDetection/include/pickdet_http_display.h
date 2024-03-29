#include "esp_http_server.h"
#include "esp_timer.h"
#include "pickdet_motion.h"

//define desired AP SSID and password 
#define WIFI_SSID    "ESPEYE_AP"
#define WIFI_PASSWORD   "1234567890"
//define desired IP for stream. Access EXAMPLE_IP_ADDR + "/stream"
#define EXAMPLE_IP_ADDR            "192.168.4.1"

#define PART_BOUNDARY "123456789000000000000987654321" //user-defined boundary used by server to distinguish payload it receives
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

esp_err_t stream_handler(httpd_req_t *req);
void pickdet_http_main();
