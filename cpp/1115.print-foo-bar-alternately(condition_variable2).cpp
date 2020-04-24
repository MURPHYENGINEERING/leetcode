/*
 * @lc app=leetcode id=1115 lang=cpp
 *
 * [1115] Print FooBar Alternately
 */

#include <memory>
#include <functional>
#include <mutex>
#include <condition_variable>

// @lc code=start
class FooBar final
{
private:
  enum WorkType { Foo, Bar };
  using work_item = std::function<void()>;

public:
  FooBar(int n)
      : n(n), next_work_type(WorkType::Foo)
  {
  }

  void foo(const work_item printFoo)
  {
    do_work_and_switch(WorkType::Foo, printFoo, WorkType::Bar);
  }

  void bar(const work_item printBar)
  {
    do_work_and_switch(WorkType::Bar, printBar, WorkType::Foo);
  }

private:
  void do_work_and_switch(const WorkType type, const work_item& work_item, const WorkType then_type)
  {
    std::unique_lock lock(work_mtx);
    for (unsigned int i = 0; i < n; ++i)
    {
      if (next_work_type != type)
        work_barrier.wait(lock, [&] { return next_work_type == type; });

      work_item();
      next_work_type = then_type;
      work_barrier.notify_all();
    }
  }

  const unsigned int n;
  // Synchronize access to counters
  std::mutex work_mtx;
  // Latch each thread to enforce ordering
  std::condition_variable work_barrier;

  WorkType next_work_type;
};
// @lc code=end
