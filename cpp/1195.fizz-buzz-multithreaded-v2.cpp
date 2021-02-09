/*
 * @lc app=leetcode id=1195 lang=cpp
 *
 * [1195] Fizz Buzz Multithreaded
 */

// @lc code=start
#include <mutex>
#include <condition_variable>

class FizzBuzz {
private:
  struct Task {
    using task_type = char;
    
    static constexpr task_type FIZZ = 1 << 0;
    static constexpr task_type BUZZ = 1 << 1;
    static constexpr task_type FIZZBUZZ = FIZZ | BUZZ;
    
    template<typename T>
    bool assign(T value) {
      std::lock_guard lock(mtx_task);
      
      task = 
        Task::NONE
        | (value % 3 == 0) * Task::FIZZ
        | (value % 5 == 0) * Task::BUZZ;
      
      if (task == Task::NONE)
        return false;
      
      on_available.notify_all();

      return true;
    }
    
    void complete() {
      std::lock_guard lock(mtx_task);
      task = NONE; 
      on_completed.notify_all();
    }
    
    void close() { 
      std::lock_guard lock(mtx_task);
      task = FINISHED; 
      on_available.notify_all();
    }
    
    void await_completion() {
      std::unique_lock lock(mtx_task);
      on_completed.wait(lock, [&] { return task == NONE; });  
    }
    
    bool await_next(const task_type of_type) {
      std::unique_lock lock(mtx_task);
      on_available.wait(lock, [&] { 
        return task == of_type || task == FINISHED; 
      });
      return task != FINISHED;
    }

  private:
    static constexpr task_type NONE = 0;
    static constexpr task_type FINISHED = FIZZBUZZ + 1;
    
    task_type task = NONE;
    std::mutex mtx_task;
    std::condition_variable on_available;
    std::condition_variable on_completed;
  };

  
  const int n;
  unsigned int i = 0;
  
  Task task;

public:
  FizzBuzz(const int n) : n(n) {
  }

  void number(const function<void(int)> printNumber) {
    while (++i <= n) {
      if (!task.assign(i)) {
        printNumber(i);
        continue;
      }
      
      task.await_completion();
    }
    
    task.close();
  }
  
  void work_loop(const Task::task_type my_task, const function<void()> do_work) {
    while (task.await_next(my_task)) {     
      do_work();
      task.complete();
    }
  }
  
  void fizz(const function<void()> printFizz) { work_loop(Task::FIZZ, printFizz); }
  void buzz(const function<void()> printBuzz) { work_loop(Task::BUZZ, printBuzz); }
  void fizzbuzz(
    const function<void()> printFizzBuzz) { work_loop(Task::FIZZBUZZ, printFizzBuzz); }
};
// @lc code=end

