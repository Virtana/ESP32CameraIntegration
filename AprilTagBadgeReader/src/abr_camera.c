
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "abr_camera.h"
#include "abr_apriltags.h"
#include "abr_pin_map.h"

#include "esp_system.h"
#include "sensor.h"


esp_err_t initialize_camera()
{

    //Assign Camera pins to config
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;

    config.pin_pwdn = PIN_PWDN;
    config.pin_reset = PIN_RESET;
    config.pin_xclk = PIN_XCLK;
    config.pin_sscb_sda = PIN_SIOD;
    config.pin_sscb_scl = PIN_SIOC;
    config.pin_d0 = PIN_Y2;
    config.pin_d1 = PIN_Y3;
    config.pin_d2 = PIN_Y4;
    config.pin_d3 = PIN_Y5;
    config.pin_d4 = PIN_Y6;
    config.pin_d5 = PIN_Y7;
    config.pin_d6 = PIN_Y8;
    config.pin_d7 = PIN_Y9;
    config.pin_vsync = PIN_VSYNC;
    config.pin_href = PIN_HREF;
    config.pin_pclk = PIN_PCLK;

    config.xclk_freq_hz = 20000000; //20MHz

    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_VGA; // VGA = 640x480  (see sensors.h)

    /* USING GRAYSCALE:
        FRAMESIZE_QVGA (320X240px) - stable| inconsistent detection when image is taken at an angle. Takes ~1 second per detection

        FRAMESIZE_VGA (640x480px) - seems to be stable..............| , detection is significantly more consistent than QVGA. Takes ~4 seconds per detection.
            FRAMESIZE_VGA is only stable if alignment is set such that no line padding is added to image_u8 created (i.e. stride = width) (see abr_apriltags.c).
            For default alignment (line padding added), program crashes (Guru Meditation Error: Core  0 panic'ed (StoreProhibited). Exception was unhandled.)

        FRAMESIZE > VGA - Instant crash! (Guru Meditation Error: Core  0 panic'ed (StoreProhibited). Exception was unhandled.)
    */

    //jpeg_quality determines the amount of loss during jpeg compression. This value ranges from 0-63, where 0 is almost lossless (highest quality, highest image size)
    //63 gives the lowest quality and lowest image size
    config.jpeg_quality = 25;
    config.fb_count = 1;

    /* ON ESP-EYE, IO13, IO14 is designed for JTAG by default,
     * to use it as generalized input,
     * firstly declare it as pullup input */
    gpio_config_t conf;
    conf.mode = GPIO_MODE_INPUT;
    conf.pull_up_en = GPIO_PULLUP_ENABLE;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.intr_type = GPIO_INTR_DISABLE;
    conf.pin_bit_mask = 1LL << 13;
    gpio_config(&conf);
    conf.pin_bit_mask = 1LL << 14;
    gpio_config(&conf);


    //Initialize the camera driver ... see esp_camera.c
    esp_err_t error = esp_camera_init(&config);

    if(error!=ESP_OK)
    {
        configPRINTF(("Camera initialization failed\n"));
        return error;
    }
    
    configPRINTF(("Camera successfully initialized\n"));
    return error;
}


void capture_image(void* pvParameters)
{
    while(true)
    {
        camera_fb_t* fb = esp_camera_fb_get();

        if(fb == NULL)
        {
            configPRINTF(("Failed to capture image\n"));
            break;
        }
        
        detect_apriltags(fb,(QueueHandle_t*)pvParameters);

        esp_camera_fb_return(fb);

        vTaskDelay(pdMS_TO_TICKS(750));

    }

    vTaskDelete(NULL);
}
