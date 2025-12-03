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