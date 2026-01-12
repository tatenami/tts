#ifndef SCHED_H
#define SCHED_H

#include <coroutine>
#include <queue>
#include <unordered_map>
#include <array>
#include <memory>
#include "task_types.hpp"

namespace tts 
{

class TaskIDAllocator {
  private:
  TaskID next_id{0};
  std::queue<TaskID> free_ids_;
  
  public:
  TaskID allocate();
  void free(TaskID id);
};

// スケジューラは singleton
class Scheduler {
  using HandlerAddr = void*;

 private:
  Scheduler() = default;
  ~Scheduler() = default;

  TaskID running_task_id_;
  std::array<std::unique_ptr<TaskControlBlock>, MAX_TASK_NUM> tcb_list_;
  std::unordered_map<std::string, TaskID> name_to_id_;
  std::unordered_map<HandlerAddr, TaskID> handler_to_id_;
  std::queue<TaskControlBlock*> ready_queue_;
  std::queue<TaskControlBlock*> finish_queue_;
  TaskIDAllocator id_allocator_;

  TaskControlBlock& getTCBFromHandler(std::coroutine_handle<> h) {
    TaskID id = handler_to_id_.at(h.address());
    return *(tcb_list_[id]);
  }

  TaskControlBlock& getTCBFromName(std::string name) {
    TaskID id = name_to_id_.at(name);
    return *(tcb_list_[id]);
  }

 public:
  Scheduler(const Scheduler&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;
  Scheduler(const Scheduler&&) = delete;
  Scheduler& operator=(Scheduler&&) = delete;

  static Scheduler& instance() {
    static Scheduler instance;
    return instance;
  }

  TaskID getTaskID(std::string task_name) {
    return name_to_id_[task_name];
  }

  TaskID getTaskID(std::coroutine_handle<> h) {
    return handler_to_id_[h.address()];
  }

  TaskState getTaskState(TaskID id) {
    return tcb_list_.at(id).get()->state;
  }


  TaskID registerTask(std::string name, Task&& task);
  void enqueueReady(std::coroutine_handle<> h);
  void enqueueFinish(std::coroutine_handle<> h);
  bool requestSuspend(TaskID id);
  bool requestResume(TaskID id);
  void registerSleep();
  void removeReady(std::coroutine_handle<> h);
  void cleanTask(std::coroutine_handle<> h);
  void run();
};

}

#endif