#ifndef HID_H
#define HID_H

#include <LUFA/Drivers/USB/USB.h>

enum OUT_REPORT_TYPE {
  SET_ICON,
  SET_VOLUME,
  SET_APP,
};

typedef struct {
  uint8_t index;
  uint16_t x0;
  uint16_t x1;
  uint8_t y0;
  uint8_t y1;
  uint32_t data_len;
} ImageData;

void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_StartOfFrame(void);
void EVENT_USB_Device_ControlRequest(void);

void Bulk_Task(void);
void HID_Task(void);

#endif // !HID_H
