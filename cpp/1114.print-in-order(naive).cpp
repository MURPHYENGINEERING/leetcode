/*
 * @lc app=leetcode id=1114 lang=cpp
 *
 * [1114] Print in Order
 */

// Modern C++ function pointers (since C++11)
#include <functional>

// Since C++20 use std::atomic<T>::wait
// #include <atomic>

// C++11 through C++17, lock mutex and wait on CV
#include <mutex>
#include <condition_variable>

// @lc code=start
class Foo
{
public:
  Foo() : released_count(0)
  {
  }

  void first(const std::function<void()> printFirst)
  {
    // Don't repeat yourself:
    // All of our locking and condition checking is in one place.
    // We give ourselves fewer opportunities for mistakes and fewer
    // places to look if there's an issue.
    wait_for_then_inc(0, printFirst);
  }

  void second(const std::function<void()> printSecond)
  {
    wait_for_then_inc(1, printSecond);
  }

  void third(const std::function<void()> printThird)
  {
    wait_for_then_inc(2, printThird);
  }

private:
  // Do use function names to make behavior clear:
  // wait for a given count, then increment. Notice that the caller would
  // be wrong to make any assumption about the value of released_count when 
  // the function argument is called; we should document that, if not with the function name.
  // Ideally we'd instead use an ordering mechanism that doesn't depend on the
  // caller knowing about the count--I may submit another solution along those lines,
  // but it would buy that opacity at the cost of performance and simplicity.
  void wait_for_then_inc(const std::uint_fast8_t count, const std::function<void()>& then)
  {
    {
      // std::condition_variable::wait needs to be able to lock and unlock;
      // std::lock_guard takes a lock immediately and doesn't release it until
      // its destructor runs (when it goes out of scope).
      // std::unique_lock facilitates manual locking and unlocking. It's movable,
      // but not copyable, so it can't be accidentally duplicated and used to deadlock.
      std::unique_lock lock(stoplight_mtx);
      // Naive to the implementation of std::condition_variable::wait,
      // we run our own predicate loop.
      stoplight.wait(lock, [=] { return released_count == count; });

      then();

      // This operation is atomic while we hold the lock.
      ++released_count;
    }
    // A notified thread will immediately try to acquire the lock,
    // so we release the lock before notifying to minimize contention.
    // See https://stackoverflow.com/a/17102100/9945076
    stoplight.notify_all();
  }

  std::mutex stoplight_mtx;
  std::condition_variable stoplight;
  // Unlike in Java, the volatile keyword has no multi-threading semantics in C++.
  std::uint_fast8_t released_count;
};
// @lc code=end
