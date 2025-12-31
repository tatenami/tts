#include <cstdio>
#include "tts.hpp"

using namespace tts::task;

Task task1() {
  for (int i = 0; i < 10; i++) {
    std::printf("[task1] num: %d\n", i);
    co_await std::suspend_always{};
  }
}

Task task2() {
  for (int i = 0; i < 10; i++) {
    std::printf("[task2] num: %d\n", i);
    co_await std::suspend_always{};
  }
}

int main() {
  auto t1 = task1();
  auto t2 = task2();

  while (!t1.handler.done() || !t2.handler.done()) {
    t1.handler.resume();
    t2.handler.resume();
  }

  return 0;
}