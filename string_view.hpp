#pragma once

#include <string_view>
#include <charconv>
#include <concepts>
#include <cassert>

/** Helper functions for string_views
 */
namespace string_view {

  /** A small helper for parsing integral numbers efficiently from a string_view. 
   *  If the number cannot be parsed from the given string an assertion is raised.
   */
  template<std::integral T>
  T into(std::string_view sv, int base = 10) {
    T number;
    auto result = std::from_chars(sv.data(), sv.data() + sv.size(), number, base);
    assert(result.ec != std::errc::invalid_argument && result.ec != std::errc::result_out_of_range); // conversion should not fail
    return number;
  }
  
}

