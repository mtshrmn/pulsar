#ifndef HID_H
#define HID_H

#include <LUFA/Drivers/USB/USB.h>

void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_StartOfFrame(void);
void EVENT_USB_Device_ControlRequest(void);
void CALLBACK_HID_Device_ProcessHIDReport(
    USB_ClassInfo_HID_Device_t *const HIDInterfaceInfo, const uint8_t ReportID,
    const uint8_t ReportType, const void *ReportData,
    const uint16_t ReportSize);

bool CALLBACK_HID_Device_CreateHIDReport(
    USB_ClassInfo_HID_Device_t *const HIDInterfaceInfo, uint8_t *const ReportID,
    const uint8_t ReportType, void *ReportData, uint16_t *const ReportSize);

bool CALLBACK_HIDParser_FilterHIDReportItem(
    HID_ReportItem_t *const CurrentItem);

void HID_Task(void);
void setPotentiometerValue(uint8_t value);

#endif // !HID_H
