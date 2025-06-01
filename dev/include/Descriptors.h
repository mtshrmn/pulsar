#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include <LUFA/Drivers/USB/USB.h>

#define VID 0x1209
#define PID 0x2711

#define HID_IN_EPADDR (ENDPOINT_DIR_IN | 1)
#define HID_OUT_EPADDR (ENDPOINT_DIR_OUT | 2)
#define BULK_OUT_EPADDR (ENDPOINT_DIR_OUT | 3)

// Endpoint sizes
#define HID_EPSIZE 8
#define BULK_EPSIZE 64

// Interface numbers
#define INTERFACE_ID_HID 0
#define INTERFACE_ID_BULK 1

// HID Report Descriptor size
#define HID_REPORT_DESC_SIZE 32 // Adjust as needed

typedef struct {
  USB_Descriptor_Configuration_Header_t Config;

  // HID Interface
  USB_Descriptor_Interface_t HID_Interface;
  USB_HID_Descriptor_HID_t HID_HIDDescriptor;
  USB_Descriptor_Endpoint_t HID_ReportINEndpoint;
  USB_Descriptor_Endpoint_t HID_ReportOUTEndpoint;

  // Bulk OUT Interface
  USB_Descriptor_Interface_t Bulk_Interface;
  USB_Descriptor_Endpoint_t Bulk_OUTEndpoint;
} USB_Descriptor_Configuration_t;

// Descriptor enums
enum InterfaceDescriptors_t {
  INTERFACE_DESCRIPTOR_HID = INTERFACE_ID_HID,
  INTERFACE_DESCRIPTOR_BULK = INTERFACE_ID_BULK,
};

enum StringDescriptors_t {
  STRING_ID_Language = 0,
  STRING_ID_Manufacturer = 1,
  STRING_ID_Product = 2,
};

uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint16_t wIndex,
                                    const void **const DescriptorAddress)
    ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);

#endif // !DESCRIPTORS_H
