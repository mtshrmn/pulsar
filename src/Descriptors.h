#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include <LUFA/Drivers/USB/USB.h>

#define VID 0x1209
#define PID 0x2711

#define IN_EPADDR (ENDPOINT_DIR_IN | 1)
#define OUT_EPADDR (ENDPOINT_DIR_OUT | 2)
#define EPSIZE 8
#define REPORT_SIZE 8

typedef struct {
  USB_Descriptor_Configuration_Header_t Config;
  USB_Descriptor_Interface_t Interface;
  USB_HID_Descriptor_HID_t Descriptor;
  USB_Descriptor_Endpoint_t ReportINEndpoint;
  USB_Descriptor_Endpoint_t ReportOUTEndpoint;
} USB_Descriptor_Configuration_t;

enum InterfaceDescriptors_t {
  INTERFACE_ID = 0, /**< GenericHID interface descriptor ID */
};

enum StringDescriptors_t {
  STRING_ID_Language = 0,
  STRING_ID_Manufacturer = 1,
  STRING_ID_Product = 2,
};

#endif // !DESCRIPTORS_H
