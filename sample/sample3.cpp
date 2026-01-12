#include <cstdio>
#include "../tts.hpp"

using namespace tts;

Task task1() {
  for (int i = 1; i <= 10; i++) {
    std::printf("[task1] num: %d\n", i);

    usleep(300000);
    co_await yield();
  }
}

Task task2() {
  for (int i = 1; i <= 10; i++) {
    std::printf("[task2] num: %d\n", i);

    if (i == 5) {
      task_suspend("task1");
    }

    usleep(300000);
    co_await yield();
  }

  task_resume("task1");
}

int main() {
  create_task("task1", task1);
  create_task("task2", task2);

  start_scheduler();

  return 0;
}