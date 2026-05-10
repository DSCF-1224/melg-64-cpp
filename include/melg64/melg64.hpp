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

#include <cstddef>
// https://cppreference.com/cpp/header/cstddef
// https://cpprefjp.github.io/reference/cstddef.html

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

template <std::size_t __NN>
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
  using FuncPtr = melg64::result_type (melg_base::*)() noexcept;

  static constexpr inline std::size_t NN = __NN;

  std::size_t i_;

  melg64::result_type state_[NN];

  /**
   * @brief extra state variable
   */
  melg64::result_type lung_;

  FuncPtr next_;
};

using melg607 = melg_base<9>;

using melg1279 = melg_base<19>;

using melg2281 = melg_base<35>;

using melg4253 = melg_base<66>;

using melg11213 = melg_base<175>;

using melg19937 = melg_base<311>;

using melg44497 = melg_base<695>;

}  // namespace melg64

static_assert(std::uniform_random_bit_generator<melg64::melg607>);
static_assert(std::uniform_random_bit_generator<melg64::melg1279>);
static_assert(std::uniform_random_bit_generator<melg64::melg2281>);
static_assert(std::uniform_random_bit_generator<melg64::melg4253>);
static_assert(std::uniform_random_bit_generator<melg64::melg11213>);
static_assert(std::uniform_random_bit_generator<melg64::melg19937>);
static_assert(std::uniform_random_bit_generator<melg64::melg44497>);

#define MELG64_H_
#endif /* MELG64_H_ */
