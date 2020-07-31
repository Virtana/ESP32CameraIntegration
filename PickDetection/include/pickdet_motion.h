//change according to resolution chosen in pickdet_camera.h
#define WIDTH 320
#define HEIGHT 240
#define BLOCK_SIZE 10
#define W (WIDTH / BLOCK_SIZE)
#define H (HEIGHT / BLOCK_SIZE)
#define BLOCK_DIFF_THRESHOLD 0.15
#define IMAGE_DIFF_THRESHOLD 0.15

void print_frame();
void app_capture_still();
bool app_motion_detect();
void app_update_frame();
void pickdet_motion_detect();
