#ifndef TASK_TYPES_H
#define TASK_TYPES_H

#include <cstdint>
#include "tts_config.h"

namespace tts 
{

using TaskID = uint32_t;
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
