#include "abr_display_image.h"
#include "abr_apriltags.h"
#include "esp_log.h"

static const char* TAG = "TAG_DISP_IMG";

httpd_handle_t camera_httpd = NULL;

esp_err_t stream_images_handler(httpd_req_t *req)
{
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len;
    uint8_t * _jpg_buf;
    char * part_buf[64];
    static int64_t last_frame = 0;
    if(!last_frame) {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        return res;
    }

    while(true)
    {
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
            break;
        }

        if(fb->format != PIXFORMAT_JPEG)
        {
            configPRINTF(("Can only display JPEG\n"));
            return ESP_FAIL;
        }

        _jpg_buf_len = fb->len;
        _jpg_buf = fb->buf;

        //DETECT APRILTAGS IN IMAGE
        detect_apriltags(fb);

        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);

            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(fb->format != PIXFORMAT_JPEG){
            free(_jpg_buf);
        }

        esp_camera_fb_return(fb);

        if(res != ESP_OK){
            break;
        }

        int64_t fr_end = esp_timer_get_time();
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        ESP_LOGI(TAG, "MJPG: %uKB %ums (%.1ffps)",
            (uint32_t)(_jpg_buf_len/1024),
            (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time);

        
    }

    last_frame = 0;
    return res;
}

void display_image_initialize()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    static bool started = false;


    httpd_uri_t stream_images_uri = 
    {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_images_handler,
        .user_ctx = NULL
    };

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);

    //This will only run on first call of function.
    if (started==false && httpd_start(&camera_httpd, &config) == ESP_OK)
    {
        started = true;
        configPRINTF(("Started Server\n"));
        httpd_register_uri_handler(camera_httpd, &stream_images_uri);
    }

}
