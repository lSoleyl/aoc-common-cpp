#pragma once

#include <cstdint>
#include <cmath>

// Helper functions for common math tasks
namespace math {
  /** Returns the number of digits in the passed number (must be a positive number)
   */
  int digits(int64_t number) {
    return static_cast<int>(std::log10(number) + 1);
  }

  /** Returns a vector of all the number's digits in order
   */
  std::vector<int> allDigits(int64_t number) {
    std::vector<int> digits;
    // Run through the loop at least once to return {0} when passing 0 instead of an empty vector
    do {
      auto div = std::div(number, 10LL);
      digits.push_back(static_cast<int>(div.rem));
      number = div.quot;
    } while (number != 0);

    std::reverse(digits.begin(), digits.end());
    return digits;
  }

  template<typename T>
  concept IntegralReference = std::is_integral_v<std::remove_reference_t<T>>;


  template<typename T>
  concept IntegerRange = requires(T&& range) {
    range.begin();
    range.end();
    { *range.begin() } -> IntegralReference;
  };


  /** The inverse to allDigits() - constructs a number from the integer digits between 0-9
   *  The digits may be passed in as ints or as char (where they will be interpreted as ASCII digits)
   */
  template<IntegerRange Range>
  int64_t fromDigits(Range&& range) {
    int64_t number = 0;
    for (auto digit : range) {
      number *= 10;
      if constexpr (std::is_same_v<decltype(digit), char>) {
        // Passed a character (interpret as ASCII digit)
        number += (digit - '0');
      } else {
        // Regular integer digit
        number += digit;
      }
    }
    return number;
  }


  /** Calculate power of 10 for huge numbers (without risk of double conversion errors)
   *  by performing repeated multiplication (not super efficient)
   */
  constexpr int64_t power10(int exponent) {
    int64_t result = 1;
    for (; exponent > 0; --exponent) {
      result *= 10;
    }
    return result;
  }

  /** Divide by the same number multiple times. Equivalent to: number / (divisor ^ divisorExponent)
   */
  int64_t divPower(int64_t number, int64_t divisor, int divisorExponent) {
    for (; divisorExponent > 0; --divisorExponent) {
      number /= divisor;
    }
    return number;
  }

  /** Left shifts a base 10 number by the specified amount of digits filling up with zeroes
   */
  int64_t leftShift(int64_t number, int digits) {
    return number * power10(digits+1);
  }

  /** Right shifts a base 10 number by the specified amount of digits
   */
  int64_t rightShift(int64_t number, int digits) {
    return number / power10(digits+1);
  }

  /** Appends a digit between 0 and 9 to the given number and returns the result
   */
  int64_t appendDigit(int64_t number, int digit) {
    return number * 10 + digit;
  }

  /** Appends a digit between 0 and 9 (as character) to the given number and returns the result
   */
  int64_t appendDigit(int64_t number, char digit) {
    return appendDigit(number, static_cast<int>(digit - '0'));
  }


  /** Splits a base 10 number in such a way that second will contain up to 
   *  suffixDigits and all remaining digits will be placed in first.
   *  split(123456789, 4) -> [12345, 6789]
   *  If the number has fewer than suffixDigits first will be zero
   */
  std::pair<int64_t, int64_t> split(int64_t number, int suffixDigits) {
    auto result = std::div(number, power10(suffixDigits + 1));
    return std::make_pair(result.quot, result.rem);
  }
}