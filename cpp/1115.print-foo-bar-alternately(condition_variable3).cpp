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

  struct WorkCycleStep {
    WorkCycleStep(const WorkType type) : work_type(type) {}

    WorkType work_type;
    std::shared_ptr<WorkCycleStep> next_step;
  };

public:
  FooBar(int n)
      : n(n)
  {
    add_work_step(WorkType::Foo);
    add_work_step(WorkType::Bar);
  }

  void foo(const work_item printFoo)
  {
    do_work_when_ready(WorkType::Foo, printFoo, n);
  }

  void bar(const work_item printBar)
  {
    do_work_when_ready(WorkType::Bar, printBar, n);
  }

private:
  void do_work_when_ready(
    const WorkType type, 
    const work_item& work_item,
    const unsigned int num_repetitions
  ) {
    std::unique_lock lock(work_mtx);

    for (unsigned int i = 0; i < num_repetitions; ++i)
    {
      if (next_step->work_type != type)
        work_barrier.wait(lock, [&] { return next_step->work_type == type; });

      do_work_now(work_item);
    }
  }

  void do_work_now(const work_item& work_item) {
    work_item();
    // Go to next step
    next_step = next_step->next_step.get();
    work_barrier.notify_all();
  }

  void add_work_step(const WorkType type) {
    if (!work_cycle) {
      // Add the first step such that it points back to itself to form a cycle
      work_cycle = std::make_shared<WorkCycleStep>(type);
      work_cycle->next_step = work_cycle;
      next_step = work_cycle.get();
    } else {
      // Define the last step as the one whose next step is the beginning of the cycle
      WorkCycleStep* last_step = work_cycle.get();
      while (last_step->next_step != work_cycle) {
        last_step = last_step->next_step.get();
      }
      // Add a new step to the end of the cycle and make it point back to the beginning
      auto new_step = std::make_shared<WorkCycleStep>(type);
      new_step->next_step = work_cycle;
      last_step->next_step = new_step;
    }
  }

  const unsigned int n;

  // Circular list specifying which work may be completed at each step
  std::shared_ptr<WorkCycleStep> work_cycle;
  // Iterator for the above. We're not conforming to the standard C++ iterators because
  // first == last defines an empty sequence, whereas for us that's a cycle of radius one.
  const WorkCycleStep* next_step;

  std::mutex work_mtx;
  std::condition_variable work_barrier;
};
// @lc code=end
