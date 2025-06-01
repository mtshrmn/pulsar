#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <stdlib.h>

#include "daemon.h"
#include "log.h"

void daemon_run(void) {
  int ret;
  int len;
  libusb_device_handle *handle = NULL;
  uint8_t hid_report[HID_REPORT_SIZE] = {0};

  LOGI("starting daemon");

  handle = libusb_open_device_with_vid_pid(NULL, VID, PID);
  if (handle == NULL) {
    LOGE("failed opening HID device");
    return;
  }

  if (libusb_kernel_driver_active(handle, INTERFACE) == 1) {
    libusb_detach_kernel_driver(handle, INTERFACE);
  }

  ret = libusb_claim_interface(handle, INTERFACE);
  if (ret != 0) {
    LOGE("libusb failed to claim interface");
    libusb_close(handle);
    return;
  }

  ret = libusb_interrupt_transfer(handle, HID_EP_OUT, hid_report,
                                  sizeof(hid_report), &len, 1000);
  if (ret != 0 || len != sizeof(hid_report)) {
    LOGE("error sending HID report [%d] - %s", ret, libusb_error_name(ret));
    goto out;
  }

out:
  libusb_release_interface(handle, INTERFACE);
  libusb_close(handle);
  LOGI("stopping daemon");
  return;
}
