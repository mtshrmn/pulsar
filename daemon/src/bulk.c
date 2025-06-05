#include "bulk.h"
#include "log.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define HIGH(x) ((uint8_t)((x) >> 8))
#define LOW(x) ((uint8_t)((x)))

RGB565 rgb888_to_rgb565(RGB888 *color) {
  RGB565 rgb565 = {
      .low = ((color->r >> 3) << 3) | color->g >> 5,
      .high = (color->g & 0xE0) | color->b >> 3,
      // .high = 0xE0 | color->b >> 3,
  };
  return rgb565;
}

int bulk_send_image(libusb_device_handle *handle, const char *image_path,
                    uint16_t x0, uint8_t y0) {
  int width;
  int height;
  int channels;
  ImageHeader header;

  uint8_t *data;
  uint8_t *rgb565_data;

  int ret;
  int len;

  data = stbi_load(image_path, &width, &height, &channels, 3);
  LOGI("loaded image %s (w*h=%d*%d, c=%d)", image_path, width, height,
       channels);

  header = (ImageHeader){
      .x0 = x0,
      .x1 = x0 + width - 1,
      .y0 = y0,
      .y1 = y0 + height - 1,
      .data_len = 2 * width * height,
  };

  rgb565_data = malloc(header.data_len);
  if (rgb565_data == NULL) {
    LOGE("failed to allocate rgb565_data of size %d", header.data_len);
    return 1;
  }

  LOGI("converting RGB888 data to RGB565");
  for (int i = 0; i < width * height; ++i) {
    RGB565 rgb565_pixel = rgb888_to_rgb565((RGB888 *)(data + i * 3));
    *(RGB565 *)(rgb565_data + i * 2) = rgb565_pixel;
  }

  LOGI("sending header over bulk");
  ret = libusb_bulk_transfer(handle, BULK_EP_OUT, (uint8_t *)&header,
                             sizeof(header), &len, 1000);

  if (ret != 0) {
    LOGE("bulk transfer failed - %s", libusb_error_name(ret));
    return ret;
  }

  if (len != sizeof(header)) {
    LOGE("failed to send %lu bytes, sent %d instead", sizeof(header), len);
    return 1;
  }

  LOGI("sending image data over bulk");

  ret = libusb_bulk_transfer(handle, BULK_EP_OUT, rgb565_data, header.data_len,
                             &len, 1000);

  if (ret != 0) {
    LOGE("bulk transfer failed - %s", libusb_error_name(ret));
    return ret;
  }

  if (len != (int)header.data_len) {
    LOGE("failed to send %u bytes, sent %d instead", header.data_len, len);
    return 1;
  }

  return 0;
}
