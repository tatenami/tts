#include <linux/types.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/slab.h> // kalloc, kfree
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/poll.h>

#include <linux/version.h>
#include <linux/errno.h>
#include <linux/printk.h>

// for timer
#include <linux/hrtimer.h>
#include <linux/ktime.h>

// for fifo
#include <linux/kfifo.h>

#include "../tts_ioctl.h"

MODULE_LICENSE("Dual BSD/GPL");

// static struct hrtimer timer;

#define task_id_t uint8_t

struct proc_timer_ctx;

struct task_timer {
  struct hrtimer timer;
  task_id_t task_id;
  struct proc_timer_ctx *ctx;
};

struct proc_timer_ctx {
  struct task_timer timers[MAX_TASK_NUM];
  DECLARE_KFIFO(expired_ids, task_id_t, 16);
  spinlock_t fifo_lock;
  wait_queue_head_t wq;
};

/* Module Finctions */
static enum hrtimer_restart timer_callback(struct hrtimer *t) {
  // hrtimerに対応したタスク対応タイマー構造体の逆引き
  struct task_timer *ttimer = container_of(t, struct task_timer, timer);
  struct proc_timer_ctx* ctx = ttimer->ctx;

  pr_info("tts_timer: callback (task_id=%d)\n", ttimer->task_id);

  // kfifo にタスクID push
  spin_lock(&(ctx->fifo_lock));

  if (!kfifo_put(&(ctx->expired_ids), ttimer->task_id)) {
    pr_info("tts_timer: kfifo is full\n");
  }

  spin_unlock(&(ctx->fifo_lock));

  wake_up_interruptible(&(ctx->wq));

  int len = kfifo_len(&(ctx->expired_ids));
  pr_info("tts_timer: kfifo elements = %d\n", len);
  pr_info("tts_timer: finish timer callback (task_id=%d)\n", ttimer->task_id);
  // kfree(ttimer);

  return HRTIMER_NORESTART;
}

struct proc_timer_ctx* proc_timer_ctx_create(void) {
  struct proc_timer_ctx *ctx = kzalloc(sizeof(struct proc_timer_ctx), GFP_KERNEL);

  INIT_KFIFO(ctx->expired_ids);
  spin_lock_init(&(ctx->fifo_lock));
  init_waitqueue_head(&(ctx->wq));

  for (size_t i = 0; i < MAX_TASK_NUM; i++) {
    ctx->timers[i].task_id = i;
    ctx->timers[i].ctx = ctx;
    hrtimer_setup(&(ctx->timers[i].timer), timer_callback, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
  }

  return ctx;
}

void proc_timer_ctx_destroy(struct proc_timer_ctx *ctx) {
  for (size_t i = 0; i < MAX_TASK_NUM; i++) {
    hrtimer_cancel(&(ctx->timers[i].timer));
  }

  kfree(ctx);
}

static int tts_timer_open(struct inode *inode, struct file *file) {
  pr_info("tts_timer: (pid=%d) open\n", current->pid);

  struct proc_timer_ctx *ctx = proc_timer_ctx_create();
  if (ctx == NULL) {
    return -ENOMEM;
  }

  file->private_data = ctx;
  return 0;
}

static long tts_timer_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
  pr_info("tts_timer: (pid=%d) ioctl\n", current->pid);

  struct proc_timer_ctx *ctx = (struct proc_timer_ctx *)(file->private_data);

  switch (cmd) {
    case TTS_SLEEP_REQ_CMD: {
      ioctl_sleep_req_arg req_arg;
      int ret = copy_from_user(&req_arg, (ioctl_sleep_req_arg __user *)arg, sizeof(req_arg));
      if (ret) {
        pr_info("tts_timer: failed to copy user data\n");
        return 0;
      }

      if (req_arg.task_id >= MAX_TASK_NUM) {
        return -EINVAL;
      }

      task_id_t task_id = req_arg.task_id;
      uint64_t  sleep_ns = req_arg.sleep_ns;
      struct hrtimer *timer = &(ctx->timers[task_id].timer);
      ktime_t sleep_time = ktime_set(0, sleep_ns);

      pr_info("tts_timer: (pid=%d) Add timer (task_id=%d, ns=%lu)\n", current->pid, task_id, sleep_ns);
      hrtimer_start(timer, sleep_time, HRTIMER_MODE_REL);
      break;
    }
    case TTS_GET_EXPIRED_COUNT_CMD: {
      uint32_t count;
      unsigned long flags;

      spin_lock_irqsave(&(ctx->fifo_lock), flags);
      count = kfifo_len(&(ctx->expired_ids));
      spin_unlock_irqrestore(&(ctx->fifo_lock), flags);

      if(copy_to_user((void __user *)arg, &count, sizeof(count))) {
        return -EFAULT;
      }

      break;
    }
  }

  return 0;
}

static __poll_t tts_timer_poll(struct file *file, struct poll_table_struct *ptable) {
  pr_info("tts_timer: (pid=%d) poll\n", current->pid);

  __poll_t mask = 0;
  struct proc_timer_ctx *ctx = (struct proc_timer_ctx *)(file->private_data);

  poll_wait(file, &(ctx->wq), ptable);

  unsigned long flags;
  spin_lock_irqsave(&(ctx->fifo_lock), flags);
  if (!kfifo_is_empty(&(ctx->expired_ids))) {
    mask |= (POLLIN | POLLRDNORM);
  }  
  spin_unlock_irqrestore(&(ctx->fifo_lock), flags);
  
  return mask;
}

ssize_t tts_timer_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
  pr_info("tts_timer: (pid=%d) read\n", current->pid);

  struct proc_timer_ctx *ctx = (struct proc_timer_ctx *)(file->private_data);
  uint32_t copied_size;

  if (count <= 0) {
    return -EINVAL;
  }

  size_t max = count - (count % sizeof(task_id_t));

  unsigned long flags;
  spin_lock_irqsave(&(ctx->fifo_lock), flags);

  if (kfifo_is_empty(&(ctx->expired_ids))) {
    spin_unlock_irqrestore(&(ctx->fifo_lock), flags);
    return -EAGAIN;
  }

  kfifo_to_user(&(ctx->expired_ids), buf, max, &copied_size);

  spin_unlock_irqrestore(&(ctx->fifo_lock), flags);

  pr_info("tts_timer: (pid=%d) copy to user (fifo ids) size: %d\n", current->pid, copied_size);

  return copied_size;
}

static int tts_timer_release(struct inode *inode, struct file *file) {
  pr_info("tts_timer: (pid=%d) release\n", current->pid);

  proc_timer_ctx_destroy((struct proc_timer_ctx *)(file->private_data));
  return 0;
}

static struct file_operations fops = {
  .owner = THIS_MODULE,
  .open  = tts_timer_open,
  .unlocked_ioctl = tts_timer_ioctl,
  .poll = tts_timer_poll,
  .read = tts_timer_read,
  .release = tts_timer_release,
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

  return 0;
}

static void tts_timer_exit(void) {
  pr_info("tts_timer: exit.\n");
  misc_deregister(&misc_dev);
}

module_init(tts_timer_init);
module_exit(tts_timer_exit);
