#include <coroutine>
#include <exception>

using namespace std;

namespace tts {
  namespace task {
    struct Task {
      struct TaskPromise;
      using handle = std::coroutine_handle<TaskPromise>;
      handle handler;

      Task(handle h): handler{h} {}
      Task(Task&& t): handler{t.handler} { t.handler = nullptr; }

      ~Task() {
        if (handler) {
          handler.destroy();
        }
      }
    };

    struct Task::TaskPromise {
      Task get_return_object() { 
        return Task {
          std::coroutine_handle<TaskPromise>::from_promise(*this)
        }; 
      };
      std::suspend_always initial_suspend() { return {}; };
      std::suspend_always final_suspend() noexcept { return {}; };
      void return_void() {};
      void unhandled_exception() { std::terminate(); }
    };
  }
}

template <typename... Args>
struct std::coroutine_traits<tts::task::Task, Args...> {
  using promise_type = tts::task::Task::TaskPromise;
};

