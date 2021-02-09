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
    static constexpr char NONE = 0;
    static constexpr char FIZZ = 1 << 0;
    static constexpr char BUZZ = 1 << 1;
    static constexpr char FIZZBUZZ = FIZZ | BUZZ;
    static constexpr char FINISHED = 1 << 2;
  };


  const int n;
  unsigned int i = 0;

  std::mutex mtx_work;
  // Wake up workers from the delegation thread
  std::condition_variable work_available;
  // Release the delegation thread without waking up other workers
  std::condition_variable work_completed;

  // Specifies which worker to activate
  char task = Task::NONE;

public:
  FizzBuzz(int n) : n(n) {
  }

  void number(function<void(int)> printNumber) {
    std::unique_lock lk(mtx_work);
    
    while (++i <= n) {
      task = Task::NONE;
      task |= (i % 3 == 0) * Task::FIZZ;
      task |= (i % 5 == 0) * Task::BUZZ;

      if (task == Task::NONE) {
        printNumber(i);
        continue;
      }

      work_available.notify_all();
      work_completed.wait(lk, [&] { return task == Task::NONE; });
    }
    
    task = Task::FINISHED;
    work_available.notify_all();
  }
  
  void work_loop(const char my_task, const function<void()> result) {
    std::unique_lock lk(mtx_work);
    
    while (1) {
      work_available.wait(lk,  [&] { return task == my_task || task == Task::FINISHED; });
      if (task == Task::FINISHED) return;
      
      result();
      task = Task::NONE;
      work_completed.notify_all();
    }
  }
  
  void fizz(function<void()> printFizz) {
    work_loop(Task::FIZZ, printFizz);
  }

  void buzz(function<void()> printBuzz) {
    work_loop(Task::BUZZ, printBuzz);
  }

  void fizzbuzz(function<void()> printFizzBuzz) {
    work_loop(Task::FIZZBUZZ, printFizzBuzz);
  }
};
// @lc code=end

