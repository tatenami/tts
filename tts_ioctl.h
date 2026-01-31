#ifndef TTS_IOCTL_H
#define TTS_IOCTL_H

#include <linux/ioctl.h>

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

typedef struct {
  uint8_t task_id;
  uint64_t sleep_ns;
} ioctl_sleep_arg;

#define TTS_IOCTL_MAGIC 0xAA

#define TTS_ADD_SLEEP_TASK_CMD _IOW(TTS_IOCTL_MAGIC, 1, ioctl_sleep_arg)

#endif // TTS_IOCTL_H