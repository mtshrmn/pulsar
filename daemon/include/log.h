#ifndef LOG_H
#define LOG_H

#ifdef LOGLEVEL_INFO
#define LOGE(fmt, ...)                                                         \
  printf("[\033[0;31mERROR\033[0m]: " fmt "\n", ##__VA_ARGS__)
#define LOGI(fmt, ...) printf("[INFO]: " fmt "\n", ##__VA_ARGS__)
#endif

#ifdef LOGLEVEL_ERROR
#define LOGE(fmt, ...)                                                         \
  printf("[\033[0;31mERROR\033[0m]: " fmt "\n", ##__VA_ARGS__)
#define LOGI(fmt, ...)
#endif

#endif
