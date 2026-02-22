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

typedef uint8_t task_id_t;
typedef unsigned long expired_bitmap_t;

static_assert((sizeof(unsigned long) * 8) >= MAX_TASK_NUM);

typedef struct {
  task_id_t task_id;
  uint64_t sleep_ns;
} ioctl_sleep_req_arg;

#define TTS_IOCTL_MAGIC 0xAA

#define TTS_SLEEP_REQ_CMD _IOW(TTS_IOCTL_MAGIC, 1, ioctl_sleep_req_arg)
#define TTS_HAS_EXPIRED_CMD _IOR(TTS_IOCTL_MAGIC, 2, uint8_t) // bool
#define TTS_ABORT_SLEEP_CMD _IOW(TTS_IOCTL_MAGIC, 3, task_id_t) // task_id
#define TTS_GET_EXPIRED_BITMAP_CMD _IOR(TTS_IOCTL_MAGIC, 4, expired_bitmap_t)

#endif // TTS_IOCTL_H
