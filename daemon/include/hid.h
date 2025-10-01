#ifndef HID_H
#define HID_H

#include "common/protocol.h"
#include "log.h" // UNUSED macro
#include <stdbool.h>
#include <stdint.h>

#define RING_BUFFER_SIZE 16

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RGB888;

typedef struct {
  uint8_t low;  // 5r + 3 g
  uint8_t high; // 3g + 5b
} RGB565;

typedef struct {
  uint8_t ring_buffer[RING_BUFFER_SIZE][HID_EPSIZE];
  size_t read_head;
  size_t write_head;
  bool is_ready_to_dequeue;
} HIDQueue;

int hid_enqueue_report(uint8_t *report, size_t size);
int hid_enqueue_report_and_wait(uint8_t *report, size_t size);
int hid_dequeue_report(void);
void hid_report_queue_mark_ready(void);
int bulk_send_image(uint8_t index, const char *image_path, uint16_t x,
                    uint8_t y);

#endif // !HID_H
