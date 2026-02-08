#ifndef TTS_IOCTL_H
#define TTS_IOCTL_H

#include <linux/ioctl.h>

#ifdef __KERNEL__

#include <linux/types.h>
#define TASK_ID_TYPE uint8_t

#else
#include <stdint.h>
#endif

#include "tts_config.h"

typedef struct {
  uint8_t task_id;
  uint64_t sleep_ns;
} ioctl_sleep_req_arg;

#define TTS_IOCTL_MAGIC 0xAA

#define TTS_SLEEP_REQ_CMD _IOW(TTS_IOCTL_MAGIC, 1, ioctl_sleep_req_arg)
#define TTS_GET_EXPIRED_COUNT_CMD _IOR(TTS_IOCTL_MAGIC, 2, uint8_t)

#endif // TTS_IOCTL_H