// Pre calculated camera bounds values to speed up entity_on_screen()
extern int32_t camera_xmin, camera_ymin;
extern uint32_t camera_xsize, camera_ysize;

typedef struct {
    int32_t x;          // 4 bytes
    int32_t y;          // 4 bytes
    int32_t x_mark;     // 4 bytes
    int32_t y_mark;     // 4 bytes
    int32_t x_offset;   // 4 bytes
    int32_t y_offset;   // 4 bytes
    int16_t x_shifted;  // 2 bytes
    int16_t y_shifted;  // 2 bytes
    Entity *target;     // 2 bytes (SNES pointers are 16-bit in near memory)
} cameraStruct;

extern cameraStruct camera;

// Initialize the camera with default values (upper left, no target)
void camera_init();
// Center camera directly on a specific point
// This does not redraw the tilemap, call stage_draw_screen() manually after
void camera_set_position(int32_t x, int32_t y);
// Shake camera for a specified number of frames
void camera_shake(uint16_t time);
// Per frame update for camera, moves toward the target entity and scrolls the tilemap
void camera_update();
