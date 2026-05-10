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

template <std::size_t __NN, std::size_t __MM, melg64::result_type __MATRIX_A>
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

  static constexpr inline melg64::result_type mag01[2] = {
      static_cast<melg64::result_type>(0), __MATRIX_A};

  static constexpr inline std::size_t MM = __MM;

  static constexpr inline std::size_t NN = __NN;

  std::size_t i_;

  melg64::result_type state_[NN];

  /**
   * @brief extra state variable
   */
  melg64::result_type lung_;

  FuncPtr next_;

  constexpr void initialize_member_i(void) noexcept {
    this->i_ = static_cast<std::size_t>(0);
  }

  constexpr void initialize_member_state(const melg64::result_type seed) {
    constexpr melg64::result_type multiplier =
        static_cast<melg64::result_type>(6364136223846793005UL);

    this->state_[0] = seed;

    std::size_t i = static_cast<std::size_t>(1);

    for (; i < this->NN; i++) {
      this->state_[i] =
          multiplier * melg_base::mat3pos(62, this->state_[i - 1]) + this->i_;
    }

    this->lung_ =
        multiplier * melg_base::mat3pos(62, this->state_[i - 1]) + this->i_;
  }

  static constexpr melg64::result_type mat3neg(
      const int t, const melg64::result_type v) noexcept {
    return v ^ (v << t);
  }

  static constexpr melg64::result_type mat3pos(
      const int t, const melg64::result_type v) noexcept {
    return v ^ (v >> t);
  }
};

using melg607 = melg_base<9, 5, 0x81f1fd68012348bcUL>;

using melg1279 = melg_base<19, 7, 0x1afefd1526d3952bUL>;

using melg2281 = melg_base<35, 17, 0x7cbe23ebca8a6d36UL>;

using melg4253 = melg_base<66, 29, 0xfac1e8c56471d722UL>;

using melg11213 = melg_base<175, 45, 0xddbcd6e525e1c757UL>;

using melg19937 = melg_base<311, 81, 0x5c32e06df730fc42UL>;

using melg44497 = melg_base<695, 373, 0x4fa9ca36f293c9a9UL>;

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
