// The zero thread counts n to limit and delegates to the appropriate thread
// We want to avoid lock contention as much as possible so the zero thread will
// only notify the thread whose turn it is, instead of notifying all and having the
// threads predicate on the condition (they still predicate for spurious wakes).

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
      
      // We implement the branching logic here so that we only ever wake a thread
      // that will pass its predicate and do work.
      if (is_turn(Turn::Even)) {
        even_barrier_.notify_one();
      } else {
        odd_barrier_.notify_one();
      }
      
      // Since C++20 we can wait on the atomic changing
      // Release lock so the turn can proceed
      zero_barrier_.wait(lock, [&] { return turn_is_completed; });
      
      ++n_;
      turn_is_completed = false;
    }
    
    // should_quit is true so release both threads
    even_barrier_.notify_one();
    odd_barrier_.notify_one();
  }

  void even(function<void(int)> print_number) {
    take_turns_repeatedly(Turn::Even, even_barrier_, print_number);
  }

  void odd(function<void(int)> print_number) {
    take_turns_repeatedly(Turn::Odd, odd_barrier_, print_number);
  }
  
private:
  enum Turn { Even, Odd };
  // Each thread checks this condition so encapsulate the logic
  bool is_turn(Turn turn) const {
    return !turn_is_completed && ((turn == Turn::Even) ? (n_ % 2 == 0) : (n_ % 2 != 0));
  }
  // Name this logic to show intent in the thread conditions
  bool should_quit() const {
    return n_ > turn_limit_;
  }
  
  void take_turns_repeatedly(
    const Turn turn_to_take, 
    std::condition_variable& barrier,
    const std::function<void(int)> take_turn
  ) {
    std::unique_lock lock(turn_mtx_);

    while (!should_quit()) {
      // Release lock so zero thread can delegate
      barrier.wait(lock, [&] { return should_quit() || is_turn(turn_to_take); });
      if (should_quit()) return;
      
      take_turn(n_);
      
      // Since C++20 we only need to set an atomic<bool> to notify
      turn_is_completed = true;
      zero_barrier_.notify_one();
    }
  }
  
  
  const int turn_limit_;
  // We output the turn number and the first turn outputs 1 so there is no zeroth turn.
  int n_ = 1;  
  bool turn_is_completed{false};
  
  // All threads share the two state fields and need to be synchronized.
  std::mutex turn_mtx_;
  // The delegation thread blocks here while waiting for the delegated thread to finish
  std::condition_variable zero_barrier_;
  // Each thread blocks separately so the appropriate one can be woken by the delegator.
  // If we had more work conditions we'd be looking at a priority queue or map for these,
  // depending on what those conditions were.
  std::condition_variable even_barrier_;
  std::condition_variable odd_barrier_;
};
