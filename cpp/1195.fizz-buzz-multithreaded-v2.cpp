/*
 * @lc app=leetcode id=1195 lang=cpp
 *
 * [1195] Fizz Buzz Multithreaded
 */

// @lc code=start
#include <mutex>        // for unique_lock
#include <shared_mutex> // for shared_lock
#include <condition_variable>

struct Task {
  using type = int;

  static constexpr type
    FIZZ = 1 << 0,
    BUZZ = 1 << 1,
    FIZZBUZZ = FIZZ | BUZZ;

  Task() : task(NONE) {}

  template<typename T>
  bool assign(const T i) {
    task = Task::NONE
          | (i % 3 == 0) * Task::FIZZ
          | (i % 5 == 0) * Task::BUZZ;
    if (task == NONE)
      return false;

    on_available.notify_all();
    return true;
  }

  void complete() {
    task = Task::COMPLETED;
    on_completion.notify_all();
  }

  void terminate() {
    task = TERMINATE;
    on_available.notify_all();
  }

  template<typename T>
  void await_completion(T& lock) {
    on_completion.wait(lock, [&] { return task == COMPLETED; });
  }

  template<typename T>
  bool await_next(T& lock, const type wait_for) {
    on_available.wait(lock,  [&] { return task == wait_for || task == Task::TERMINATE; });
    return task != Task::TERMINATE;
  }
    
private:
  static constexpr type
    NONE = 0,
    COMPLETED = NONE,
    TERMINATE = -1;

  type task;

  std::condition_variable_any on_available;
  std::condition_variable_any on_completion;
};
// ============ -Task ============


class FizzBuzz {
private:
  // iterating i in 1..n
  const int n;
  unsigned int i = 0;
  
  // ============== Shared state =============
  Task task;
  std::shared_mutex task_mtx;
  // =========================================
  
  void master_loop(const function<void(int)> default_work) {
    std::unique_lock lock(task_mtx);
    
    while (++i <= n) {
      if (task.assign(i))
        task.await_completion(lock);
      else
        default_work(i);
    }

    task.terminate();
  }
  
  void work_loop(const Task::type my_task, const function<void()> do_work) {
    std::shared_lock lock(task_mtx);
    
    while (task.await_next(lock, my_task)) {
      do_work();
      task.complete();
    }
  }

public:
  FizzBuzz(int n) : n(n) {}
  
  // Master thread
  void number(function<void(int)> print) { master_loop(print); }
  
  // Workers
  void fizz(function<void()> print) { work_loop(Task::FIZZ, print); }
  void buzz(function<void()> print) { work_loop(Task::BUZZ, print); }
  void fizzbuzz(function<void()> print) { work_loop(Task::FIZZBUZZ, print); }
};
// @lc code=end

