#include "daemon.h"
#include "common/protocol.h"
#include "log.h"
#include "pulseaudio.h"
#include <libusb-1.0/libusb.h>

unsigned char buf[HID_EPSIZE];

static void transfer_cb(struct libusb_transfer *transfer) {
  if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
    LOGE("transfer error [%d]", transfer->status);
    libusb_free_transfer(transfer);
  }

  // TODO: do something with transfer
  LOGI("recieved transfer of length [%d]:", transfer->actual_length);
  for (int i = 0; i < transfer->actual_length; ++i) {
    printf("%02x ", transfer->buffer[i]);
  }
  printf("\n");
  libusb_submit_transfer(transfer);
}

int daemon_run(libusb_device_handle *handle) {
  int ret;
  struct libusb_transfer *transfer;

  LOGI("starting daemon");

  if (libusb_kernel_driver_active(handle, INTERFACE_ID_HID) == 1) {
    LOGI("detaching kernel driver");
    libusb_detach_kernel_driver(handle, INTERFACE_ID_HID);
  }

  LOGI("attempting to claim interface");
  ret = libusb_claim_interface(handle, INTERFACE_ID_HID);
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
  libusb_release_interface(handle, INTERFACE_ID_HID);
  libusb_close(handle);
  return ret;
}
