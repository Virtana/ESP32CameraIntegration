#include "FreeRTOS.h"
#include "task.h"
#include "esp_log.h"
#include "math.h"
#include "pickdet_motion.h"
#include "FreeRTOS.h"
#include "task.h"

static const char *TAG = "motion_detect";

uint16_t current_frame[H][W] = { 0 };
uint16_t prev_frame[H][W] = { 0 };

void app_downsample(camera_fb_t *fb) {
    // set all 0s in current frame
    //Initializing pixel values in fb to prepare for image capture
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            current_frame[y][x] = 0;   


    // down-sample image in blocks
    for (uint32_t i = 0; i < WIDTH * HEIGHT; i++) {
        const uint16_t x = i % WIDTH;
        const uint16_t y = floor(i / WIDTH);
        const uint8_t block_x = floor(x / BLOCK_SIZE);
        const uint8_t block_y = floor(y / BLOCK_SIZE);
        const uint8_t pixel = fb->buf[i];
        esp_camera_fb_return(fb);
        // const uint16_t current = current_frame[block_y][block_x];

        // average pixels in block (accumulate)
        current_frame[block_y][block_x] += pixel;
    }

    // average pixels in block (rescale)
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            current_frame[y][x] /= BLOCK_SIZE * BLOCK_SIZE;
    
    // ESP_LOGI(TAG, "Downsampling picture...");
}


bool app_motion_detect() {
    uint16_t changes = 0;
    const uint16_t blocks = (WIDTH * HEIGHT) / (BLOCK_SIZE * BLOCK_SIZE);

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            float current = current_frame[y][x];
            float prev = prev_frame[y][x];
            float delta = abs(current - prev) / prev;

            if (delta >= BLOCK_DIFF_THRESHOLD) {
                changes += 1;
            }
        }
    }

    ESP_LOGI(TAG,"Changed %d out of %d",changes, blocks);PRIVATE AFR::mqtt
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            prev_frame[y][x] = current_frame[y][x];
        }
    }
}

void pickdet_motion_detect(camera_fb_t *fb){
    app_downsample(fb);
    if(app_motion_detect())ESP_LOGE(TAG, "Motion detected!");
    app_update_frame();
    ESP_LOGI(TAG, "=============================");
}

void pickdet_motion_solo(){
    while(true){
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb)
        {
            ESP_LOGE(TAG, "Camera capture failed");
            break;
        }
        else{
            app_downsample(fb);
            if(app_motion_detect())ESP_LOGE(TAG, "Motion detected!");
            app_update_frame();
        }
    }
}

void pickdet_motion_detect(){
    while (true)
    {
        app_capture_still();
        if(app_motion_detect())ESP_LOGE(TAG, "Motion detected!");
        app_update_frame();
        //vTaskDelay(100 / portTICK_RATE_MS);
        // ESP_LOGI(TAG, "=============================");
    }
}
