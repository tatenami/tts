#ifndef AWAITER_H
#define AWAITER_H

#include "debug.hpp"
#include "sched.hpp"
#include <coroutine>

namespace tts {

struct TaskAwaiterBase {
  bool await_ready() noexcept { return false; } // false 時，await したら常時 suspend
  void await_resume() noexcept {}
  // await_suspend は各 Awaiter で実装し，
  // 返り値は true にして，コルーチンを suspend する
};

/* yiled 用 */
struct TaskYieldAwaiter: TaskAwaiterBase {
  bool await_suspend(std::coroutine_handle<> h) noexcept {
    LOG_PRINT("[awaiter(y)] suspend and enqueue ready\n");

    Scheduler::instance().enqueueReady(h);
    return true;
  }
};

/* suspend (自タスク) 用 */
struct TaskSuspendAwaiter: TaskAwaiterBase {
  bool await_suspend(std::coroutine_handle<> h) noexcept {
    LOG_PRINT("[awaiter(s)] suspend and self-suspend\n");

    Scheduler &sched = Scheduler::instance();
    task_id_t id = sched.getTaskID(h);
    sched.requestSuspend(id);
    return true;
  }
};

/* スリープ用 */
struct TaskSleepAwaiter: TaskAwaiterBase {
  uint64_t sleep_ns = 0;

  bool await_suspend(std::coroutine_handle<> h) noexcept {
    LOG_PRINT("[awaiter(s)] sleep and euqueue block\n");

    Scheduler::instance().requestSleep(h, sleep_ns);
    return true;
  }
};

// タスク終了時用
struct TaskFinalAwaiter: TaskAwaiterBase {
  bool await_suspend(std::coroutine_handle<> h) noexcept {
    LOG_PRINT("[awaiter(f)] suspend and enqueue finish\n");

    Scheduler::instance().enqueueFinish(h);
    return true;
  }
};

} // namespace tts
#endif