#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#ifndef ENDPOINT_DIR_IN
#define ENDPOINT_DIR_IN 0x80
#endif

#ifndef ENDPOINT_DIR_OUT
#define ENDPOINT_DIR_OUT 0
#endif

#define VID 0x1209
#define PID 0x2711

#define HID_IN_EPADDR (ENDPOINT_DIR_IN | 1)
#define HID_OUT_EPADDR (ENDPOINT_DIR_OUT | 2)
#define BULK_OUT_EPADDR (ENDPOINT_DIR_OUT | 3)

#define HID_EPSIZE 8
#define BULK_EPSIZE 64

#define INTERFACE_ID_HID 0
#define INTERFACE_ID_BULK 1

typedef struct {
  uint8_t index;
  uint16_t x0;
  uint16_t x1;
  uint8_t y0;
  uint8_t y1;
  uint32_t data_len;
} ImageData;

typedef enum {
  OUT_REPORT_TYPE_VOLUME_INC,
  OUT_REPORT_TYPE_VOLUME_DEC,
  OUT_REPORT_TYPE_SET_VOLUME,
} OUT_REPORT_TYPE;

// both input and output will work with same reports.
typedef struct {
  uint8_t index;
  uint8_t report_type;
  uint8_t volume;
} HIDReport;

_Static_assert(sizeof(HIDReport) <= HID_EPSIZE, "HIDReport too big");

#endif // !PROTOCOL_H
