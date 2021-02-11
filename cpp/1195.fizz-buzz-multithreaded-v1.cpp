/*
 * @lc app=leetcode id=1195 lang=cpp
 *
 * [1195] Fizz Buzz Multithreaded
 */

// @lc code=start
#include <mutex>        // for unique_lock
#include <shared_mutex> // for shared_lock
#include <condition_variable>

class FizzBuzz {
private:
  // We'll classify each number i based on which condition it satisfies;
  // then assign a task to activate the correct thread.
  struct Task {
    using type = unsigned int;       

    static constexpr type
      NONE = 0,
      COMPLETED = NONE,
      FIZZ = 1 << 0,
      BUZZ = 1 << 1,
      FIZZBUZZ = FIZZ | BUZZ,
      // There are no more numbers to classify (signal termination).
      TERMINATE = FIZZBUZZ + 1;
  };

  // iterating i in 1..n
  const int n;
  unsigned int i = 0;
  
  // ============== Shared state =============
  // - written by master, read by all workers;
  // - written by one worker (reset), read by master.
  // You'll often see "volatile" used to try to make writes visible to other
  // threads; this is a misunderstanding of the guarantees made by volatile.
  //  Writes will be visible because we're locking on a mutex. Volatile does not
  // intentionally offer any thread synchronization. 
  Task::type task = Task::NONE;
  // =========================================

  // Synchronize access to the task state:
  // - workers may not read until the master has assigned a task for i.
  // - Master may not continue to the next number until a worker has reset the task.
  // A shared mutex allows worker threads to lock it simultaneously without blocking
  // each other. We can do this because we know that exactly one thread will continue
  // with work while the others only need to read the task and go back to waiting.
  //  The master thread will still take a unique lock, which blocks all of the workers;
  // conversely each of the workers will block the master.
  std::shared_mutex mtx_task;
  
  // Workers wait here until the master thread classifies the current
  // number and sets the task.
  //  When notified, all workers wake up and check the task; 
  // the worker who owns that task becomes the active worker,
  // and the others go back to waiting.
  // 
  // the *_any variant lets us wait on shared_mutex, at the possible expense 
  // of some platform optimizations.
  std::condition_variable_any workers;
  
  // Master thread waits here until the active worker notifies that the task
  // has been reset. We notify completion on its own barrier so we don't
  // wake up workers unnecessarily.
  std::condition_variable_any master;
  
  
  // This loop iterates and classifies the number i.
  // It either handles the default case, outputting the number, or it delegates
  // to one of the worker threads and waits for them to finish.
  void master_loop(const function<void(int)> default_work) {
    // We take a unique lock on the shared state so that no worker may proceed
    // until we're ready, and we don't proceed until all workers are ready.
    //  We minimize locking and unlocking by holding the lock continuously
    // unless waiting for a worker.
    std::unique_lock task_lock(mtx_task);
    
    while (++i <= n) {
      // FIZZBUZZ is defined as FIZZ | BUZZ, so this handles all three conditions.
      task = Task::NONE
            | (i % 3 == 0) * Task::FIZZ
            | (i % 5 == 0) * Task::BUZZ;

      if (task == Task::NONE) {
        default_work(i);
      } else {
        // Wake up workers to check task.
        // -- Race/Deadlock risk --
        // A worker could process the task, reset it, and notify before we're
        // listening--causing a deadlock here.
        //  We hold a unique lock on the task and release it by waiting, 
        // so no worker will proceed until we're ready.
        workers.notify_all();
        master.wait(task_lock, [&] { return task == Task::COMPLETED; });
      }
    }
    
    // Signal termination and wake all threads.
    // -- Race/Deadlock risk --
    // If we notified before a worker started waiting then we'll exit this thread
    // and deadlock while the worker waits for more tasks.
    //  We hold a unique lock and release it by returning, so we won't get here
    // until all the workers are waiting.
    task = Task::TERMINATE;
    workers.notify_all();
  }
  
  
  void work_loop(const Task::type my_task, const function<void()> do_work) {
    // Block the master from writing, but don't block other readers
    std::shared_lock task_lock(mtx_task);
    
    while (1) {
      // Wait until there's work delegated to this thread
      workers.wait(task_lock,  [&] { return task == my_task || task == Task::TERMINATE; });
      if (task == Task::TERMINATE) return;
      
      do_work();
      task = Task::COMPLETED;
      master.notify_one();
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

