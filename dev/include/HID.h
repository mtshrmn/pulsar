#ifndef HID_H
#define HID_H

#include <LUFA/Drivers/USB/USB.h>

void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_StartOfFrame(void);
void EVENT_USB_Device_ControlRequest(void);

void Bulk_Task(void);
void HID_Task(void);
void HID_ReportACK(void);

#endif // !HID_H
