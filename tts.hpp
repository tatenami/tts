#ifndef TTS_H
#define TTS_H

#include "task.hpp"
#include "sched.hpp"
#include "awaiter.hpp"
#include <string>
#include <variant>

namespace tts 
{

template <typename TaskFunc>
TaskID create_task(std::string name, TaskFunc&& task) {
  return Scheduler::instance().registerTask(name, task());
}

// scheduling
void start_scheduler();

// task operationg 
bool task_suspend(std::string task_name);
bool task_resume(std::string task_name);

// task utilities
TaskState get_task_state(std::string task_name);

// awaiter api
inline TaskYieldAwaiter yield() {
  return {};
}

inline TaskSuspendAwaiter suspend() {
  return {};
}

inline TaskSleepAwaiter sleep(uint64_t ms) {
  return TaskSleepAwaiter { .sleep_ms = ms };
}

}

#endif 