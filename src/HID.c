#include "HID.h"
#include "Descriptors.h"

static uint16_t PotentiometerValue = 0;
static uint8_t PrevHIDReportBuffer[REPORT_SIZE];
static USB_ClassInfo_HID_Device_t HIDInterface = {
    .Config =
        {
            .InterfaceNumber = INTERFACE_ID,
            .ReportINEndpoint =
                {
                    .Address = IN_EPADDR,
                    .Size = EPSIZE,
                    .Banks = 1,
                },
            .PrevReportINBuffer = PrevHIDReportBuffer,
            .PrevReportINBufferSize = sizeof(PrevHIDReportBuffer),
        },
};

bool CALLBACK_HIDParser_FilterHIDReportItem(
    HID_ReportItem_t *const CurrentItem) {
  return true;
}

bool CALLBACK_HID_Device_CreateHIDReport(
    USB_ClassInfo_HID_Device_t *const HIDInterfaceInfo, uint8_t *const ReportID,
    const uint8_t ReportType, void *ReportData, uint16_t *const ReportSize) {

  uint8_t *Report = (uint8_t *)ReportData;

  Report[0] = (PotentiometerValue >> 8) & 0xFF;
  Report[1] = PotentiometerValue & 0xFF;
  Report[2] = 0;
  Report[3] = 0;
  Report[4] = 0;
  Report[5] = 0;
  Report[6] = 0;
  Report[7] = 0;

  *ReportSize = REPORT_SIZE;
  return false;
}

void CALLBACK_HID_Device_ProcessHIDReport(
    USB_ClassInfo_HID_Device_t *const HIDInterfaceInfo, const uint8_t ReportID,
    const uint8_t ReportType, const void *ReportData,
    const uint16_t ReportSize) {}

void EVENT_USB_Device_ControlRequest(void) {
  HID_Device_ProcessControlRequest(&HIDInterface);
}

void EVENT_USB_Device_StartOfFrame(void) {
  HID_Device_MillisecondElapsed(&HIDInterface);
}

void EVENT_USB_Device_ConfigurationChanged(void) {
  Endpoint_ConfigureEndpoint(IN_EPADDR, EP_TYPE_INTERRUPT, EPSIZE, 1);
  Endpoint_ConfigureEndpoint(OUT_EPADDR, EP_TYPE_INTERRUPT, EPSIZE, 1);
}

void HID_Task(void) { HID_Device_USBTask(&HIDInterface); }

void setPotentiometerValue(uint16_t value) { PotentiometerValue = value; }
