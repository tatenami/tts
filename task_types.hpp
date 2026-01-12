#ifndef TASK_TYPES_H
#define TASK_TYPES_H

#include <cstdint>

namespace tts 
{

using TaskID = uint32_t;
static constexpr TaskID MAX_TASK_NUM = 10;
constexpr TaskID NameDuplicationErr = (MAX_TASK_NUM + 1);

enum class TaskState {
  Ready,
  Running,
  Blocked,
  Suspended,
  Finished,
};

struct Task;

struct TaskControlBlock {
  TaskID id;
  TaskState state;
  std::coroutine_handle<> handler;

  TaskControlBlock(TaskID id_, TaskState state_, std::coroutine_handle<> handler_):
    id(id_), state(state_), handler(handler_) 
    {

    }

  ~TaskControlBlock() {
    std::printf("[tcb] destructor.\n");
    if (handler) {
      handler.destroy();
    }
  }
};

}

#endif
