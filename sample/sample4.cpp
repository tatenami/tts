#include <cstdio>
#include "../tts.hpp"

using namespace tts;
using namespace std;

Task task1() {

  for (int i = 1; i <= 3; i++) { 
    printf("[task1] num: %d\n", i);
    co_await sleep_ms(5);
  }
}

Task task2() {

  for (int i = 1; i <= 3; i++) {
    printf("[task2] num: %d\n", i);
    co_await sleep_ms(2);
  }
}

int main() {
  create_task("task1", task1);
  create_task("task2", task2);

  start_scheduler();

  return 0;
}