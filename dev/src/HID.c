#include "HID.h"
#include "Descriptors.h"

extern void Bulk_ProcessData(uint8_t *buf, size_t size);
extern void HID_ProcessReport(uint8_t *buf, size_t size);
extern void HID_CreateReport(uint8_t *buf, size_t size);

void EVENT_USB_Device_ConfigurationChanged(void) {
  Endpoint_ConfigureEndpoint(HID_IN_EPADDR, EP_TYPE_INTERRUPT, HID_EPSIZE, 1);
  Endpoint_ConfigureEndpoint(HID_OUT_EPADDR, EP_TYPE_INTERRUPT, HID_EPSIZE, 1);
  Endpoint_ConfigureEndpoint(BULK_OUT_EPADDR, EP_TYPE_BULK, BULK_EPSIZE, 1);
}

void Bulk_Task(void) {
  Endpoint_SelectEndpoint(BULK_OUT_EPADDR);
  if (Endpoint_IsOUTReceived()) {
    uint8_t buffer[BULK_EPSIZE];
    uint16_t bytesReceived = Endpoint_BytesInEndpoint();

    if (bytesReceived)
      Endpoint_Read_Stream_LE(buffer, bytesReceived, NULL);

    Bulk_ProcessData(buffer, bytesReceived);
    Endpoint_ClearOUT();
  }
}

void HID_Task(void) {
  Endpoint_SelectEndpoint(HID_OUT_EPADDR);
  if (Endpoint_IsOUTReceived()) {
    uint8_t out_buf[HID_EPSIZE];
    Endpoint_Read_Stream_LE(out_buf, HID_EPSIZE, NULL);
    HID_ProcessReport(out_buf, HID_EPSIZE);
    Endpoint_ClearOUT();
  }
}

void HID_ReportACK(void) {
  Endpoint_SelectEndpoint(HID_IN_EPADDR);
  if (Endpoint_IsINReady()) {
    HIDReport report = {
        .report_type = REPORT_TYPE_ACK,
    };

    Endpoint_Write_Stream_LE(&report, sizeof(report), NULL);
    Endpoint_ClearIN();
  }
}
