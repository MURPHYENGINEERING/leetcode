/*
 * @lc app=leetcode id=1115 lang=cpp
 *
 * [1115] Print FooBar Alternately
 */

#include <functional>
#include <mutex>
#include <condition_variable>

// @lc code=start
class FooBar final {
public:
    FooBar(int n) 
        : n(n), foo_count(0), bar_count(0)
    {
    }

    void foo(const std::function<void()> printFoo) {
      std::unique_lock lock(counter_mtx);
      while (foo_count < n) {
        // Print and signal one time before waiting; whether the other thread starts
        // before or after this one, it will be woken after one foo is printed.
        // Writes are synchronized.
        printFoo();
        ++foo_count;
        // bar may wake at this point before we've begun waiting.
        // In that case it will contend for the lock until we enter the wait, then
        // it may check its predicate and continue.
        cv.notify_all();
        // The lock is released at this point, allowing bar to proceed.
        // Writes to bar_count are synchronized so the predicate is guaranteed valid.
        cv.wait(lock, [&] { return foo_count == bar_count; });
      }
    }

    void bar(const std::function<void()> printBar) {
        std::unique_lock lock(counter_mtx);
        while (bar_count < n) {
          // Don't proceed until woken up AND a foo has been written.
          // The lock is released at this point, allowing foo to proceed.
          cv.wait(lock, [&] { return  foo_count > bar_count; });
          // We now obtain the lock and are guaranteed that foo is waiting.
          // Writes are synchronized.
          printBar();
          ++bar_count;

          // foo may wake immediately, but will contend for the lock until we wait.
          cv.notify_all();
        }
    }

private:
    const unsigned int n;
    // Synchronize access to counters
    std::mutex counter_mtx;
    // Latch each thread to enforce ordering
    std::condition_variable cv;
    // Report state from each thread for ordering.
    // foo_count must be greater than bar_count before bar may be printed.
    // Both threads stop when their count is equal to n.
    unsigned int foo_count;
    unsigned int bar_count;
};
// @lc code=end

