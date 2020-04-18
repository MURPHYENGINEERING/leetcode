/**
 * @lc app=leetcode id=7 lang=cpp
 *
 * [7] Reverse Integer
 * https://leetcode.com/problems/reverse-integer/
 * 
 * Given a 32-bit signed integer, reverse digits of an integer.
 * 
 * Example 1:
 *     Input: 123
 *     Output: 321
 *     
 * Example 2:
 *     Input: -123
 *     Output: -321
 * 
 * Example 3:
 *     Input: 120
 *     Output: 21
 * Note:
 *     Assume we are dealing with an environment which could only store integers 
 *     within the 32-bit signed integer range: [−231,  231 − 1]. 
 *     For the purpose of this problem, assume that your function returns 0 when 
 *     the reversed integer overflows.
 */

/**
 * Runtime: 8 ms
 * Memory Usage: 6.5 MB
 * 
 * Naive solution:
 * 1. Stringify the integer
 * 2. Reverse characters
 * 3. Parse int string
 * 
 * Considerations:
 * - The negative sign should not be moved.
 * - What are the conditions where parsing will fail?
 *  Using std::stoi (https://en.cppreference.com/w/cpp/string/basic_string/stol)
 *  - String is not a number: throws std::invalid_argument
 *  - Result overflows: throws std::out_of_range
 */

// Integer<->String conversions
#include <string>
// Integer parsing exceptions
#include <stdexcept>

// @lc code=start
class Solution final {
public:
    int reverse(const int x) const {
        /**
         * 1. Stringify the integer
         *  We often see itoa used in c++, which on my platform is in <cstdlib>.
         * This function is not specified as part of the C++ language and may not
         * be available on other platforms. Alternative to that we see snprintf (<cstdio>)
         * as a "safe" conversion because it allows the write to be limited by the buffer size,
         * eliminating buffer overrun.
         *  Both of these utilities are incorporating C-style strings (char*) that
         * defy conventions for modern C++ in a couple of ways:
         *  1. The type has ambiguous intent; is it a pointer to a character for the
         *      callee to mutate, or is it an array of characters?
         *      char* is widely used both for strings and as a substitute for void* as 
         *      a generic pointer to memory. The usage may be obvious at the call site,
         *      but the reader is forced to infer intent from the usage.
         *  2. Memory ownership is ambiguous and must be carefully documented, which will always
         *      lead to bugs at some point (which are often difficult to identify).
         *  See https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines (SL.str.3)
         * 
         * Note that C++17 does provide std::to_chars in <charconv> which contradicts what
         * I've said about avoiding C-style strings. This is provided as a non-allocating,
         * non-throwing, locale-independent option and doesn't represent the general case.
         * See https://en.cppreference.com/w/cpp/utility/to_chars
         * 
         * Since C++11 std::to_string is a standard part of <string> and should be preferred.
         * See https://en.cppreference.com/w/cpp/string/basic_string/to_string
         */
        auto chars = std::to_string(x);
        // We'll do an in-place swap between here,
        auto forward = chars.begin();
        // and here
        // The end iterator is past the last character so we need to step back for a valid swap.
        auto backward = --chars.end();
        // Don't move the negative sign; otherwise let any invalid strings fail in int parsing.
        // Additionally it's not specified in the problem if positive signs may be included.
        // We won't assume they are, but this would be one of our first questions.
        if (*forward == '-') {
          ++forward;
        }
        // If the string has one character then no swap will occur
        while (forward < backward) {
          const char c = *backward;
          *backward = *forward;
          *forward = c;
          // Prefer to move iterators or pointers in separate statements to reduce the reader's
          // workload (vs. e.g. *forward++ = c)
          ++forward;
          --backward;
        }

        try {
          /**
           * On returning from multiple execution paths:
           * https://stackoverflow.com/questions/36707/should-a-function-have-only-one-return-statement
           * From Code Complete 17.1:
           *  Minimize the number of returns in each routine. It's harder to understand a routine if, 
           *  reading it at the bottom, you're unaware of the possibility that it returned somewhere above.
           * 
           *  Use a return when it enhances readability. In certain routines, once you know the answer, 
           *  you want to return it to the calling routine immediately. If the routine is defined in 
           *  such a way that it doesn't require any cleanup, not returning immediately means that 
           *  you have to write more code.
           * 
           * We could use a local variable for the return value and either
           *  a) set it by parsing, or
           *  b) set it to 0 by catching an overflow
           * and have a single return statement at the end of the function.
           * This presents some questions:
           * - What should the initial value be? Do we default to the 0 case and only set it on
           *    successful conversion? Are there any instances where we may not set it for reasons
           *    other than overflow? We would then return a 0 and our program would be incorrect.
           * - How do we know if we ever make a mistake and fail to set the return value? We would
           *    receive a 0 return, indicating an overflow, which is totally unrelated to our mistake.
           * 
           * By returning from the two paths we're expressing our intent as directly as possible:
           * - Only return when we meet one of the two criteria we have planned
           * - Only return a zero for the specific indication that we intend
           * - Throw a compiler error if the program may follow any other path to exit the function
           */
          return std::stoi(chars);

        } catch (const std::out_of_range& ex_overflow) {
          // The problem specifies that we return 0 if the reversed integer overflows
          return 0;

        } catch (const std::invalid_argument& ex_bad_input) {
          // invalid_argument is an std::logic_error, indicating that our program is not correct.
          throw;
        }
    }
};
// @lc code=end

