#ifndef TTS_H
#define TTS_H

#include "task.hpp"
#include "sched.hpp"
#include "awaiter.hpp"
#include <string>

namespace tts {
  TaskID register_task(std::string name, Task&& task);
}

#endif 