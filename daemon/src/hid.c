#include "hid.h"
#include "daemon.h"
#include "log.h"

extern Context daemon_ctx;

int hid_write(uint8_t *report, size_t size) {
  int len, ret;
  ret = libusb_interrupt_transfer(daemon_ctx.handle, HID_EP_OUT, report, size,
                                  &len, 1000);
  if (ret < 0) {
    LOGE("error sending hid report - %s", libusb_error_name(ret));
    return ret;
  }

  if (len != (int)size) {
    LOGE("error sending hid report, sent %d bytes but expected %zu", len, size);
    return 1;
  }

  return 0;
}
