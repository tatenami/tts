#include <cstdio>
#include "tts.hpp"

using namespace tts;

Task task1() {
  for (int i = 0; i < 5; i++) {
    std::printf("[task1] num: %d\n", i);
    co_await tts::TaskYieldAwaiter{};
  }
}

Task task2() {
  for (int i = 0; i < 5; i++) {
    std::printf("[task2] num: %d\n", i);
    co_await tts::TaskYieldAwaiter{};
  }
}

Task task3() {
  for (int i = 0; i < 5; i++) {
    std::printf("[task3] num: %d\n", i);
    co_await tts::TaskYieldAwaiter{};
  }
}

int main() {
  tts::register_task("task1", task1());
  tts::register_task("task2", task2());
  tts::register_task("task3", task3());

  tts::Scheduler::instance().run();

  return 0;
}