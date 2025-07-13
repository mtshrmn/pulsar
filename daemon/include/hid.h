#ifndef HID_H
#define HID_H

#include <stddef.h>
#include <stdint.h>

int hid_write(uint8_t *report, size_t size);
int hid_write_async(uint8_t *report, size_t size);

#endif // !HID_H
