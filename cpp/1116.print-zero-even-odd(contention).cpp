// Let's try without considering lock contention.

#include <mutex>
#include <condition_variable>


class ZeroEvenOdd final {
public:
  ZeroEvenOdd(const int n) : turn_limit_(n) {
  }

  void zero(function<void(int)> print_number) {
    std::unique_lock lock(turn_mtx_);
    
    while (!should_quit()) {
      print_number(0);
      // Let the even and odd threads duke it out for the lock.
      // The branching logic is implemented when one of the threads fails its
      // wait predicate and while the other continues.
      // So every iteration we're waking a thread that has a 0% chance of doing useful work.
      turn_barrier_.notify_all();
      // Since C++20 we can wait on the atomic changing
      turn_barrier_.wait(lock, [&] { return turn_is_completed; });

      // Next turn
      ++n_;
      turn_is_completed = false;
    }
    
    // ensure we release both threads
    turn_barrier_.notify_all();
  }

  void even(function<void(int)> print_number) {
    take_turns_repeatedly(Turn::Even, print_number);
  }

  void odd(function<void(int)> print_number) {
    take_turns_repeatedly(Turn::Odd, print_number);
  }
  
private:
  enum Turn { Even, Odd };
  // Each thread checks this condition, so encapsulate the logic
  bool is_turn(const Turn turn) const {
    return !turn_is_completed && ((turn == Turn::Even) ? (n_ % 2 == 0) : (n_ % 2 != 0));
  }
  // Name this logic to show intent in the thread conditions
  bool should_quit() const {
    return n_ > turn_limit_;
  }
  
  void take_turns_repeatedly(const Turn turn_to_take, const std::function<void(int)> take_turn) {
    std::unique_lock lock(turn_mtx_);

    while (!should_quit()) {
      // Release lock so zero can delegate
      turn_barrier_.wait(lock, [&] { return should_quit() || is_turn(turn_to_take); });
      if (should_quit()) return;
      
      take_turn(n_);
      
      // Since C++20 we only need to set an atomic<bool> to notify
      turn_is_completed = true;
      turn_barrier_.notify_all();
    }
  }
  
  
  const int turn_limit_;
  // We output the turn number and the first turn outputs 1 so there is no zeroth turn.
  int n_ = 1;
  // True when we've printed '0n' and the zero thread should resume.
  bool turn_is_completed = false;

  // All threads block here
  std::mutex turn_mtx_;
  std::condition_variable turn_barrier_;
};
