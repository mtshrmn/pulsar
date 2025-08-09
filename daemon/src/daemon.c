#include "daemon.h"
#include "log.h"
#include "pulseaudio.h"
#include <libusb-1.0/libusb.h>

static void transfer_cb(struct libusb_transfer *transfer) {
  if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
    LOGE("transfer error [%d]", transfer->status);
    libusb_free_transfer(transfer);
  }

  // TODO: do something with transfer
  libusb_submit_transfer(transfer);
}

int daemon_run(libusb_device_handle *handle) {
  int ret;
  struct libusb_transfer *transfer;
  unsigned char buf[HID_REPORT_SIZE];

  LOGI("starting daemon");

  if (libusb_kernel_driver_active(handle, INTERFACE) == 1) {
    LOGI("detaching kernel driver");
    libusb_detach_kernel_driver(handle, INTERFACE);
  }

  LOGI("attempting to claim interface");
  ret = libusb_claim_interface(handle, INTERFACE);
  if (ret != LIBUSB_SUCCESS) {
    LOGE("libusb failed to claim interface");
    ret = DAEMON_RETURN_NORETRY;
    goto out;
  }

  transfer = libusb_alloc_transfer(0);
  libusb_fill_interrupt_transfer(transfer, handle, HID_IN_EPADDR, buf,
                                 sizeof(buf), transfer_cb, NULL, 0);
  LOGI("submitting initial libusb transfer");
  ret = libusb_submit_transfer(transfer);
  if (ret < 0) {
    LOGE("error submitting initial libusb transer [%d]", ret);
    libusb_free_transfer(transfer);
    ret = DAEMON_RETURN_NORETRY;
    goto out;
  }

  ret = setup_pulseaudio_mainloop();

out:
  LOGI("stopping daemon");
  libusb_release_interface(handle, INTERFACE);
  libusb_close(handle);
  return ret;
}
