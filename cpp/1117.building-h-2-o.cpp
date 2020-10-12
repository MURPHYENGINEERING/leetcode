/*
 * @lc app=leetcode id=1117 lang=cpp
 *
 * [1117] Building H2O
 */

// @lc code=start
/*
 * We'll lean heavily on the language of the  problem here 
 * to make metaphors of the concepts of thread pooling and counting.
 *   As you read this, consider carefully how the abstraction
 * makes it more or less easy to understand what the code is
 * doing.
 *   It's often the case that the mechanics of an algorithm
 * get in the way of understanding the why of it.
 * It's also often the case, however, that excessive abstraction
 * disguises and obfuscates the mechanics of the process.
 * Each of these approaches creates unique barriers for readers,
 * and both are likely to show up in any reasonably complex code.
 *   Finding the balance between abstract and blatant is one of
 * the hard problems in writing software.
 *
 * In this example, the implementation is needlessly general
 * where the problem was specific. At the same time, it's
 * needlessly specific: the question is about water molecules,
 * but it's really about work queuing and grouping.
 */

#include <condition_variable>
#include <functional>


struct atoms {
  int hydrogens;
  int oxygens;
};

// We support arbitrary combinations of hydrogen and oxygen,
// i.e. arbitrary work groups.
struct molecules {
  static constexpr atoms water{2,1};
};

struct molecule {
  static molecule complete(const atoms& required) {  
    return molecule{required, required};
  }
  
  static molecule build(const atoms& required, atoms& pool) {
    // Assign the correct number of workers from the pool
    // to this work tracker.
    pool.hydrogens -= required.hydrogens;
    pool.oxygens -= required.oxygens;
    return molecule{required, {0,0}};
  }
  
  static bool can_build(const atoms& required, const atoms& from) {
    return from.hydrogens >= required.hydrogens && from.oxygens >= required.oxygens;
  }
  
  bool is_complete() const {
    return molecule::can_build(required, attached);
  }
  
  bool try_attach_oxygen() {
    if (attached.oxygens < required.oxygens) {
      ++attached.oxygens;
      return true;
    }
    return false;
  }
  bool try_attach_hydrogen() {
    if (attached.hydrogens < required.hydrogens) {
      ++attached.hydrogens;
      return true;
    }
    return false;
  }

  atoms required;
  atoms attached;
};


class H2O {
public:
  H2O() {

  }

  void hydrogen(function<void()> releaseHydrogen) {
    lock_type lock(m);
    // Track one new waiting thread
    arrived_hydrogen();
    // Wait until we successfully add one hydrogen to the
    // currently-building molecule. This is only possible if there
    // is a molecule being built (meaning we have sufficient threads),
    // and if the molecule needs at least one hydrogen atom. 
    // Because we're synchronized on the mutex, only one thread
    // will succeed at this while the rest wait here.
    cv.wait(lock, [&] { return partial_molecule.try_attach_hydrogen(); });
    
    // The molecule abstraction is actually just a barrier to control
    // the work done by this call. The previous statement ensures
    // that one group of work is completed at a time, beginning as
    // soon as all three of the jobs are pooled (two H and one O).
    releaseHydrogen();
    
    // This is a continual work-queuing scheme. This ensures that
    // if there is another complete group of threads waiting in the pool,
    // they begin as soon as the current group ends.
    try_start_next_molecule();
  }

  void oxygen(function<void()> releaseOxygen) {
    lock_type lock(m);
    
    arrived_oxygen();
    
    cv.wait(lock, [&] { return partial_molecule.try_attach_oxygen(); });
    
    releaseOxygen();
    
    try_start_next_molecule();
  }
  
private:
  using lock_type = std::unique_lock<std::mutex>;
  static constexpr atoms target_molecule = molecules::water;
  
  std::mutex m;
  std::condition_variable cv;
  // Waiting-worker counters, separated by job type
  atoms pool;
  // Finished-worker counters
  // This is initially set to "complete" so that no
  // work is done until sufficient workers are pooled.
  // This is just to make the logic for the first case
  // consistent with subsequent cases, where this would
  // be completed from the previous case.
  molecule partial_molecule = molecule::complete(molecules::water);

  void arrived_hydrogen() {
    // Track one new waiting thread
    ++pool.hydrogens;
    // Check if that is sufficient to release the barrier
    try_start_next_molecule();
  }
  void arrived_oxygen() {
    ++pool.oxygens;
    try_start_next_molecule();
  }
  
  void try_start_next_molecule() {
    // If there are enough threads waiting, and
    // if all of the released threads have finished,
    // release three more from the waiting pool.
    if (partial_molecule.is_complete() 
     && molecule::can_build(target_molecule, pool)) {
      partial_molecule = molecule::build(target_molecule, pool);
      // Wake up all threads. Exactly one oxygen and
      // two hydrogen threads will pass their wait,
      // controlled by the counters in the molecule field.
      cv.notify_all();
    }
  }
};
// @lc code=end

