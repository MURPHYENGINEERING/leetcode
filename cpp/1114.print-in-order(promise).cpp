/*
 * @lc app=leetcode id=1114 lang=cpp
 *
 * [1114] Print in Order
 * 
 */

#include <functional>

#include <future>

// @lc code=start
class Foo
{
public:
  Foo()
  {
  }

  void first(const std::function<void()> printFirst)
  {
    printFirst();
    first_barrier.set_value();
  }

  void second(const std::function<void()> printSecond)
  {
    first_barrier.get_future().wait();
    printSecond();
    second_barrier.set_value();
  }

  void third(const std::function<void()> printThird)
  {
    second_barrier.get_future().wait();
    printThird();
  }

private:
  std::promise<void> first_barrier;
  std::promise<void> second_barrier;
};
// @lc code=end
