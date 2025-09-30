#include "daemon.h"
#include "common/protocol.h"
#include "hid.h"
#include "log.h"
#include "pulseaudio.h"
#include "sinkctl.h"
#include <libusb-1.0/libusb.h>

extern libusb_device_handle *device_handle;

static void transfer_cb(struct libusb_transfer *transfer) {
  if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
    LOGE("transfer error [%d]", transfer->status);
    libusb_free_transfer(transfer);
    pa_mainloop_quit(transfer->user_data, 0);
    return;
  }

  HIDReport report = *(HIDReport *)transfer->buffer;
  switch (report.report_type) {
  case REPORT_TYPE_ACK:
    hid_report_queue_mark_ready();
    hid_dequeue_report();
    break;
  case REPORT_TYPE_VOLUME_INC:
    sinkctl_volume_inc(report.index);
    break;
  case REPORT_TYPE_VOLUME_DEC:
    sinkctl_volume_dec(report.index);
    break;
  default:
    LOGE("recieved unkown type of report %d", report.report_type);
    break;
  }

  libusb_submit_transfer(transfer);
}

void daemon_run(void) {
  int ret;
  struct libusb_transfer *transfer;

  LOGI("starting daemon");

  if (libusb_kernel_driver_active(device_handle, INTERFACE_ID_HID) == 1) {
    LOGI("detaching kernel driver");
    ret = libusb_detach_kernel_driver(device_handle, INTERFACE_ID_HID);
    if (ret != LIBUSB_SUCCESS) {
      LOGE("detaching kernel driver failed - %s", libusb_error_name(ret));
      goto out;
    }
  }

  LOGI("attempting to claim interface");
  ret = libusb_claim_interface(device_handle, INTERFACE_ID_HID);
  if (ret != LIBUSB_SUCCESS) {
    LOGE("libusb failed to claim interface");
    goto out;
  }

  pa_mainloop *mainloop = pa_mainloop_new();
  if (mainloop == NULL) {
    LOGE("error setting up pulseaudio mainloop");
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
    goto out;
  }

  // responsible for freeing mainloop
  setup_pulseaudio_mainloop(mainloop);

out:
  LOGI("stopping daemon");
  libusb_release_interface(device_handle, INTERFACE_ID_HID);
}
