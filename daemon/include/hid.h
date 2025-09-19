#ifndef HID_H
#define HID_H

#include "log.h" // UNUSED macro
#include <stdint.h>

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a UNUSED;
} RGB888;

typedef struct {
  uint8_t low;  // 5r + 3 g
  uint8_t high; // 3g + 5b
} RGB565;

int hid_write(uint8_t *report, size_t size);
int bulk_send_image(uint8_t index, const char *image_path, uint16_t x,
                    uint8_t y);

#endif // !HID_H
