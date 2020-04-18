/**
 * @lc app=leetcode id=1 lang=cpp
 *
 * [1] Two-Sum
 * https://leetcode.com/problems/two-sum/
 * 
 * Given an array of integers, return indices of the two numbers 
 * such that they add up to a specific target.
 * 
 * You may assume that each input would have exactly one solution, 
 * and you may not use the same element twice.
 * 
 * Example:
 * 
 *   Given nums = [2, 7, 11, 15], target = 9,
 * 
 *   Because nums[0] + nums[1] = 2 + 7 = 9, return [0, 1].
 */

/**
 * Runtime: 8 ms
 * Memory Usage: 8.5 MB
 * 
 * O(n) solution:
 * 
 * Define the complement by
 *  num + complement = target
 * 
 * For each num
 * 1. Compute complement = target - num
 * 2. Check if map contains complement in keys
 *    - Yes: return (current num index, complement index from map value)
 * 3. Insert (current num, current index)
 */

#include <vector>
#include <unordered_map>
// domain_error
#include <stdexcept>

// @lc code=start
// If a class is not designed to be extended then clarify that by marking it final
class Solution final {
public:
  // Do use const correctness: https://isocpp.org/wiki/faq/const-correctness
  // Do return stl types by value like this; the compiler can use
  // return value optimization and allocate the vector in the caller's context.
  std::vector<int> twoSum(const std::vector<int>& nums, const int target) const {    
    // The map value indexes into a vector; 
    // this is the most portable storage type for such an index.
    using index_type = std::vector<int>::size_type;
    
    // Key: an element of the input vector
    // Value: the index of that element in the vector
    // const correctness: keys are const by default,
    //  and the internal implementation doesn't allow them to be declared const here.
    std::unordered_map<int, const index_type> complements;
    
    // You'll often see nums.size() moved outside of the loop condition.
    // If that optimizes your code then the compiler will do it.
    // Prefer to give your reader fewer things to keep track of.
    for (index_type i = 0; i < nums.size(); ++i) {
      // Do use local variables like this for clarity.
      // Don't think you're optimizing anything by inlining the subtraction;
      // if the local variable can be optimized away the compiler will do it.
      const auto complement = target - nums[i];
      // One of the first questions we should ask is whether the input contains
      // negative or zero values. It's not specified, and in fact it does; but if
      // the input was only positive then we could avoid inserting or searching
      // for impossible solutions in the map with
      //  if (complement <= 0) continue;
      
      // Const correctness: mutating collections while iterating can be
      // problematic at surprising times. 
      // Clarify your intent by using const iterators.
      //  We could skip this block for the first iteration, but again
      // prefer to give your reader less to think about.
      const auto entry = complements.find(complement);
      if (entry != complements.end()) {
        // The specification asked for indices as ints, which in my case are 32 bits.
        // We used vector::size_type, which (again in my case) is an unsigned long (size_t).
        // Narrowing conversions in initialization lists are an error with 
        // -Wc++11-narrowing (clang), but all implicit narrowing should 
        // be a warning or an error.
        // Prefer C++-style casts. See https://stackoverflow.com/a/1609185 
        //  "What is the difference between static_cast<> and C style casting?"
        return {static_cast<int>(entry->second), static_cast<int>(i)};
      }
      // It's common to see unordered_map mutated with ::operator[].
      // operator[] does get-or-emplace, returning an lvalue reference
      // to the entry with the given key or default-constructing one and
      // returning a reference to that.
      //  Because we aren't asked to consider duplicates, we would rather
      // short circuit on a duplicate key than get an lvalue reference and 
      // assign a new index to it. Insert returns (iterator, bool) with
      // the value of the bool being true if the insertion was made.
      complements.insert({nums[i], i});
    }
    // The specification guarantees that exactly one solution exists among the input,
    // so our behavior at this point is not defined.
    
    //  Notice the message doesn't include any implementation details, instead only
    // using language in the context of the specification.
    // This is an issue of "undefined behavior given the specified constraints,"
    // not a problem with our code. Don't expose the caller to implementation
    // details that may be irrelevant to them and are subject to change.
    throw std::domain_error("The input doesn't contain any {a,b} such that a + b = target");
  }
};
// @lc code=end
