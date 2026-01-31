#include <linux/types.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/slab.h> // kmalloc, kfree
#include <linux/version.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/printk.h>

// for timer
#include <linux/hrtimer.h>
#include <linux/ktime.h>

#include <linux/miscdevice.h>

#include "../tts_ioctl.h"

MODULE_LICENSE("Dual BSD/GPL");

// static struct hrtimer timer;

typedef struct {
  struct hrtimer timer;
  uint8_t task_id;
} TaskTimer;

/* Module Finctions */

static enum hrtimer_restart timer_callback(struct hrtimer *t) {
  // pr_info("tts_timer: callback.\n");
  TaskTimer *ttimer = container_of(t, TaskTimer, timer);
  pr_info("tts_timer: finish timer callback (task_id=%d)\n", ttimer->task_id);

  kfree(ttimer);

  return HRTIMER_NORESTART;
}

static long tts_timer_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {

  switch (cmd) {
    case TTS_ADD_SLEEP_TASK_CMD: {
      TaskTimer *ttimer;
      ttimer = kmalloc(sizeof(*ttimer), GFP_KERNEL);
      ioctl_sleep_arg req_arg;
      copy_from_user(&req_arg, (void __user *)arg, sizeof(req_arg));

      ttimer->task_id = req_arg.task_id;

      struct hrtimer timer;
      ktime_t sleep_time = ktime_set(0, req_arg.sleep_ns);
      hrtimer_setup(&timer, timer_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

      ttimer->timer = timer;
      pr_info("tts_timer: Add timer (task_id=%d, ns=%lu)\n", ttimer->task_id, req_arg.sleep_ns);
      hrtimer_start(&(ttimer->timer), sleep_time, HRTIMER_MODE_REL);
      break;
    }
  }


  int ret = 0;
  return ret;
}

static struct file_operations fops = {
  .owner = THIS_MODULE,
  .unlocked_ioctl = tts_timer_ioctl,
};

static struct miscdevice misc_dev = {
  .minor = MISC_DYNAMIC_MINOR,
  .name = "tts_timer",
  .fops = &fops,
};

static int tts_timer_init(void) {
  pr_info("tts_timer: init.\n");
  int ret = misc_register(&misc_dev);
  if (ret) {
    pr_err("tts_timer: register failed (%d).\n", ret);
    return ret;
  }

  // timer init 
  // hrtimer_setup(&timer, timer_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

  // ktime_t interval = ktime_set(1, 0);
  // hrtimer_start(&timer, interval, HRTIMER_MODE_REL);

  return 0;
}

static void tts_timer_exit(void) {
  pr_info("tts_timer: exit.\n");
  misc_deregister(&misc_dev);
}

module_init(tts_timer_init);
module_exit(tts_timer_exit);
