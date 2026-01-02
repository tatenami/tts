#include "tts.hpp"

namespace tts {
  TaskID register_task(std::string name, Task&& task) {
    return Scheduler::instance().registerTask(name, std::move(task));
  }
}