#include "daemon.h"
#include "common/protocol.h"
#include "log.h"
#include "pulseaudio.h"
#include <libusb-1.0/libusb.h>

extern libusb_device_handle *device_handle;

static void transfer_cb(struct libusb_transfer *transfer) {
  if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
    LOGE("transfer error [%d]", transfer->status);
    libusb_free_transfer(transfer);
    pa_mainloop_quit(transfer->user_data, DAEMON_RETURN_NORETRY);
    return;
  }

  // TODO: do something with transfer
  LOGI("recieved transfer of length [%d]:", transfer->actual_length);
  for (int i = 0; i < transfer->actual_length; ++i) {
    printf("%02x ", transfer->buffer[i]);
  }
  printf("\n");
  libusb_submit_transfer(transfer);
}

int daemon_run(void) {
  int ret;
  struct libusb_transfer *transfer;

  LOGI("starting daemon");

  if (libusb_kernel_driver_active(device_handle, INTERFACE_ID_HID) == 1) {
    LOGI("detaching kernel driver");
    ret = libusb_detach_kernel_driver(device_handle, INTERFACE_ID_HID);
    LOGI("detached resulted in %d", ret);
  }

  LOGI("attempting to claim interface");
  ret = libusb_claim_interface(device_handle, INTERFACE_ID_HID);
  if (ret != LIBUSB_SUCCESS) {
    LOGE("libusb failed to claim interface");
    ret = DAEMON_RETURN_NORETRY;
    goto out;
  }

  pa_mainloop *mainloop = pa_mainloop_new();
  if (mainloop == NULL) {
    LOGE("error setting up pulseaudio mainloop");
    ret = DAEMON_RETURN_NORETRY;
    goto out;
  }

  static uint8_t buf[HID_EPSIZE];
  transfer = libusb_alloc_transfer(0);
  libusb_fill_interrupt_transfer(transfer, device_handle, HID_IN_EPADDR, buf,
                                 sizeof(buf), transfer_cb, mainloop, 0);
  LOGI("submitting initial libusb transfer");
  ret = libusb_submit_transfer(transfer);
  if (ret < 0) {
    LOGE("error submitting initial libusb transer [%d]", ret);
    libusb_free_transfer(transfer);
    ret = DAEMON_RETURN_NORETRY;
    goto out;
  }

  ret = setup_pulseaudio_mainloop(mainloop);

out:
  LOGI("stopping daemon");
  libusb_release_interface(device_handle, INTERFACE_ID_HID);
  return ret;
}
