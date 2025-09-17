#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include <LUFA/Drivers/USB/USB.h>

// must be included after LUFA
#include "common/protocol.h"

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
