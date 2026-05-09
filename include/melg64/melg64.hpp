// =============================================================================
// melg64.hpp — C++ adaptation of MELG-64
//
// Original C implementation:
//   Copyright (c) Shin Harase, Ritsumeikan University
//                 Takamitsu Kimoto
//   Non-commercial use only. For commercial use contact:
//   harase @ fc.ritsumei.ac.jp
//
// C++ adaptation:
//   Copyright (c) DSCF-1224, 2026
//   Bug reports: https://github.com/DSCF-1224/melg-64-cpp/issues
//
// Reference:
//   S. Harase and T. Kimoto, "Implementing 64-bit maximally equidistributed
//   F2-linear generators with Mersenne prime period",
//   ACM Transactions on Mathematical Software, Vol. 44, Issue 3,
//   April 2018, Article No. 30.
// =============================================================================
#ifndef MELG64_H_

#include <array>
// https://cppreference.com/cpp/header/array
// https://cpprefjp.github.io/reference/array.html

#include <concepts>
// https://cppreference.com/cpp/header/concepts
// https://cpprefjp.github.io/reference/concepts.html

#include <cstdint>
// https://cppreference.com/cpp/header/cstdint
// https://cpprefjp.github.io/reference/cstdint.html
// https://cpprefjp.github.io/reference/cstdint/uint_fast64_t.html

#include <limits>
// https://cppreference.com/cpp/header/limits
// https://cppreference.com/cpp/types/numeric_limits
// https://cppreference.com/cpp/types/numeric_limits/max
// https://cppreference.com/cpp/types/numeric_limits/min
// https://cpprefjp.github.io/reference/limits.html
// https://cpprefjp.github.io/reference/limits/numeric_limits.html
// https://cpprefjp.github.io/reference/limits/numeric_limits/max.html
// https://cpprefjp.github.io/reference/limits/numeric_limits/min.html

#include <random>
// https://cppreference.com/cpp/header/random
// https://en.cppreference.com/cpp/named_req/UniformRandomBitGenerator
// https://cppreference.com/cpp/numeric/random/uniform_random_bit_generator
// https://cpprefjp.github.io/reference/random.html
// https://cpprefjp.github.io/reference/random/uniform_random_bit_generator.html

namespace melg64 {

using result_type = std::uint_fast64_t;

static_assert(std::unsigned_integral<melg64::result_type>);

class melg_base {
 public:
  // Requirements

  using result_type = melg64::result_type;

  static constexpr melg64::result_type default_seed =
      static_cast<melg64::result_type>(19650218UL);

  /**
   * @brief Yields the smallest value that `melg_base`'s `operator()` may return
   */
  static constexpr melg64::result_type min() noexcept {
    return std::numeric_limits<melg64::result_type>::min();
  }

  /**
   * @brief Yields the largest value that `melg_base`'s `operator()` may return
   */
  static constexpr melg64::result_type max() noexcept {
    return std::numeric_limits<melg64::result_type>::max();
  }

  /**
   * @warning Placeholder for future implementation
   */
  melg64::result_type operator()() {
    return static_cast<melg64::result_type>(0);
  }

 private:
};

static_assert(std::uniform_random_bit_generator<melg64::melg_base>);

}  // namespace melg64

#define MELG64_H_
#endif /* MELG64_H_ */
