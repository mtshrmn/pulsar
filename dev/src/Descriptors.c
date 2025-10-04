#include "Descriptors.h"

const USB_Descriptor_HIDReport_Datatype_t PROGMEM HIDReportDescriptor[] = {
    0x06, 0x00, 0xFF, // Usage Page (Vendor Defined)
    0x09, 0x01,       // Usage (Vendor Usage 1)
    0xA1, 0x01,       // Collection (Application)
    0x09, 0x02,       //   Usage (Vendor Usage 2)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x08,       //   Report Count (8)
    0x81, 0x02,       //   Input (Data, Var, Abs)
    0x09, 0x03,       //   Usage (Vendor Usage 3)
    0x91, 0x02,       //   Output (Data, Var, Abs)
    0xC0              // End Collection
};

const USB_Descriptor_Device_t PROGMEM DeviceDescriptor = {
    .Header =
        {
            .Size = sizeof(USB_Descriptor_Device_t),
            .Type = DTYPE_Device,
        },
    .USBSpecification = VERSION_BCD(2, 0, 0),
    .Class = USB_CSCP_NoDeviceClass,
    .SubClass = USB_CSCP_NoDeviceSubclass,
    .Protocol = USB_CSCP_NoDeviceProtocol,

    .Endpoint0Size = FIXED_CONTROL_ENDPOINT_SIZE,
    .VendorID = VID,
    .ProductID = PID,
    .ReleaseNumber = VERSION_BCD(0, 0, 1),

    .ManufacturerStrIndex = STRING_ID_Manufacturer,
    .ProductStrIndex = STRING_ID_Product,
    .SerialNumStrIndex = NO_DESCRIPTOR,

    .NumberOfConfigurations = 1,
};

const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor = {
    .Config =
        {
            .Header =
                {
                    .Size = sizeof(USB_Descriptor_Configuration_Header_t),
                    .Type = DTYPE_Configuration,
                },
            .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
            .TotalInterfaces = 2,

            .ConfigurationNumber = 1,
            .ConfigurationStrIndex = NO_DESCRIPTOR,

            .ConfigAttributes =
                (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED),
            .MaxPowerConsumption = USB_CONFIG_POWER_MA(100),
        },

    .HID_Interface =
        {
            .Header =
                {
                    .Size = sizeof(USB_Descriptor_Interface_t),
                    .Type = DTYPE_Interface,
                },
            .InterfaceNumber = INTERFACE_ID_HID,
            .AlternateSetting = 0,

            .TotalEndpoints = 2,
            .Class = HID_CSCP_HIDClass,
            .SubClass = HID_CSCP_NonBootSubclass,
            .Protocol = HID_CSCP_NonBootProtocol,
            .InterfaceStrIndex = NO_DESCRIPTOR,
        },

    .HID_HIDDescriptor =
        {
            .Header =
                {
                    .Size = sizeof(USB_HID_Descriptor_HID_t),
                    .Type = HID_DTYPE_HID,
                },
            .HIDSpec = VERSION_BCD(1, 1, 1),
            .CountryCode = 0,
            .TotalReportDescriptors = 1,
            .HIDReportType = HID_DTYPE_Report,
            .HIDReportLength = sizeof(HIDReportDescriptor),
        },

    .HID_ReportINEndpoint =
        {
            .Header =
                {
                    .Size = sizeof(USB_Descriptor_Endpoint_t),
                    .Type = DTYPE_Endpoint,
                },
            .EndpointAddress = HID_IN_EPADDR,
            .Attributes =
                EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA,
            .EndpointSize = HID_EPSIZE,
            .PollingIntervalMS = 0x05,
        },

    .HID_ReportOUTEndpoint =
        {
            .Header =
                {
                    .Size = sizeof(USB_Descriptor_Endpoint_t),
                    .Type = DTYPE_Endpoint,
                },
            .EndpointAddress = HID_OUT_EPADDR,
            .Attributes =
                EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA,
            .EndpointSize = HID_EPSIZE,
            .PollingIntervalMS = 0x05,
        },

    .Bulk_Interface =
        {
            .Header =
                {
                    .Size = sizeof(USB_Descriptor_Interface_t),
                    .Type = DTYPE_Interface,
                },
            .InterfaceNumber = INTERFACE_ID_BULK,
            .AlternateSetting = 0,
            .TotalEndpoints = 1,
            .Class = 0xFF, // Vendor Specific
            .SubClass = 0,
            .Protocol = 0,
            .InterfaceStrIndex = NO_DESCRIPTOR,
        },

    .Bulk_OUTEndpoint =
        {
            .Header =
                {
                    .Size = sizeof(USB_Descriptor_Endpoint_t),
                    .Type = DTYPE_Endpoint,
                },
            .EndpointAddress = BULK_OUT_EPADDR,
            .Attributes =
                EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA,
            .EndpointSize = BULK_EPSIZE,
            .PollingIntervalMS = 0,
        },
};

const USB_Descriptor_String_t PROGMEM LanguageString =
    USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);

const USB_Descriptor_String_t PROGMEM ManufacturerString =
    USB_STRING_DESCRIPTOR(L"My Manufacturer");

const USB_Descriptor_String_t PROGMEM ProductString =
    USB_STRING_DESCRIPTOR(L"Volume Mixer");

uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint16_t wIndex,
                                    const void **const DescriptorAddress) {
  const uint8_t DescriptorType = (wValue >> 8);
  const uint8_t DescriptorNumber = (wValue & 0xFF);

  const void *Address = NULL;
  uint16_t Size = NO_DESCRIPTOR;

  switch (DescriptorType) {
  case DTYPE_Device:
    Address = &DeviceDescriptor;
    Size = sizeof(DeviceDescriptor);
    break;
  case DTYPE_Configuration:
    Address = &ConfigurationDescriptor;
    Size = sizeof(ConfigurationDescriptor);
    break;
  case DTYPE_String:
    switch (DescriptorNumber) {
    case STRING_ID_Language:
      Address = &LanguageString;
      Size = pgm_read_byte(&LanguageString.Header.Size);
      break;
    case STRING_ID_Manufacturer:
      Address = &ManufacturerString;
      Size = pgm_read_byte(&ManufacturerString.Header.Size);
      break;
    case STRING_ID_Product:
      Address = &ProductString;
      Size = pgm_read_byte(&ProductString.Header.Size);
      break;
    }
    break;
  case HID_DTYPE_HID:
    Address = &ConfigurationDescriptor.HID_HIDDescriptor;
    Size = sizeof(ConfigurationDescriptor.HID_HIDDescriptor);
    break;
  case HID_DTYPE_Report:
    Address = &HIDReportDescriptor;
    Size = sizeof(HIDReportDescriptor);
    break;
  }

  *DescriptorAddress = Address;
  return Size;
}
