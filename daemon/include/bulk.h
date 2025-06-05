#ifndef BULK_H
#define BULK_H

#include <libusb-1.0/libusb.h>

#define BULK_EP_OUT 0x03

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t _pad;
} RGB888;

typedef struct {
  uint8_t low;  // 5r + 3 g
  uint8_t high; // 3g + 5b
} RGB565;

typedef struct __attribute__((packed, aligned(1))) {
  uint8_t index;
  uint16_t x0;
  uint16_t x1;
  uint8_t y0;
  uint8_t y1;
  uint32_t data_len;
} ImageHeader;

int bulk_send_image(libusb_device_handle *handle, const char *image_path,
                    uint16_t x0, uint8_t y0);

#endif // !BULK_H
