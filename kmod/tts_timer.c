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

// for fifo
#include <linux/kfifo.h>

#include <linux/miscdevice.h>

#include "../tts_ioctl.h"

MODULE_LICENSE("Dual BSD/GPL");

// static struct hrtimer timer;

typedef struct {
  struct hrtimer timer;
  uint8_t task_id;
} task_timer;

typedef struct {
  task_timer timers[MAX_TASK_NUM];
  struct kfifo expired_ids;
} proc_timer_ctx;

proc_timer_ctx ctx;

/* Module Finctions */

static enum hrtimer_restart timer_callback(struct hrtimer *t) {
  // hrtimerに対応したタスク対応タイマー構造体の逆引き
  task_timer *ttimer = container_of(t, task_timer, timer);

  // kfifo にタスクID push
  int ret = kfifo_put(&(ctx.expired_ids), ttimer->task_id);
  if (ret == 0) {
    pr_info("tts_timer: kfifo is full\n");
  }

  int len = kfifo_len(&(ctx.expired_ids));
  pr_info("tts_timer: kfifo elements = %d\n", len);
  pr_info("tts_timer: finish timer callback (task_id=%d)\n", ttimer->task_id);
  // kfree(ttimer);

  return HRTIMER_NORESTART;
}

static int tts_timer_open(struct inode *inode, struct file *file) {
  pr_info("tts_timer: open tts_timer by (pid=%d)\n", current->pid);

  return 0;
}

static long tts_timer_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
  switch (cmd) {
    case TTS_SLEEP_REQ_CMD: {
      // タスク対応タイマー構造体の生成
      // task_timer *ttimer;
      // ttimer = kmalloc(sizeof(*ttimer), GFP_KERNEL);

      // ioctl_sleep_req_arg req_arg;
      // copy_from_user(&req_arg, (void __user *)arg, sizeof(req_arg));

      // ttimer->task_id = req_arg.task_id;

      // // タイマーを生成
      // struct hrtimer timer;
      // hrtimer_setup(&timer, timer_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

      // ttimer->timer = timer;
      // ktime_t sleep_time = ktime_set(0, req_arg.sleep_ns);
      // pr_info("tts_timer: Add timer (task_id=%d, ns=%lu)\n", ttimer->task_id, req_arg.sleep_ns);
      // hrtimer_start(&(ttimer->timer), sleep_time, HRTIMER_MODE_REL);

      ioctl_sleep_req_arg req_arg;
      int ret = copy_from_user(&req_arg, (void __user *)arg, sizeof(req_arg));
      if (ret) {
        pr_info("tts_timer: failed to copy user data\n");
        return 0;
      }

      struct hrtimer *timer = &(ctx.timers[req_arg.task_id].timer);
      ktime_t sleep_time = ktime_set(0, req_arg.sleep_ns);
      pr_info("tts_timer: Add timer (task_id=%d, ns=%lu)\n", req_arg.task_id, req_arg.sleep_ns);
      hrtimer_start(timer, sleep_time, HRTIMER_MODE_REL);
      break;
    }
  }

  return 0;
}

static int tts_timer_release(struct inode *inode, struct file *file) {
  // for (size_t i = 0; i < MAX_TASK_NUM; i++) {
  //   hrtimer_cancel(&(ctx.timers->timer));
  // }

  return 0;
}

static struct file_operations fops = {
  .owner = THIS_MODULE,
  .open  = tts_timer_open,
  .unlocked_ioctl = tts_timer_ioctl,
  .release = tts_timer_release
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

  // kfifo 生成
  ret = kfifo_alloc(&(ctx.expired_ids), 16, GFP_KERNEL);
  if (ret) {
    return ret;
  }

  for (size_t i = 0; i < MAX_TASK_NUM; i++) {
    ctx.timers[i].task_id = i;
    hrtimer_setup(&(ctx.timers[i].timer), timer_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
  }

  return 0;
}

static void tts_timer_exit(void) {
  kfifo_free(&(ctx.expired_ids));

  pr_info("tts_timer: exit.\n");
  misc_deregister(&misc_dev);
}

module_init(tts_timer_init);
module_exit(tts_timer_exit);
