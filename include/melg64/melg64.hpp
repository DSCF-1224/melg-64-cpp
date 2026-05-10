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

template <std::size_t __NN, std::size_t __MM, melg64::result_type __MATRIX_A,
          int __P, std::size_t __LAG1, int __SHIFT1,
          melg64::result_type __MASK1>
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

  static constexpr inline std::size_t LAG1 = __LAG1;

  static constexpr inline melg64::result_type mag01[2] = {
      static_cast<melg64::result_type>(0), __MATRIX_A};

  static constexpr inline melg64::result_type MASK1 = __MASK1;

  static constexpr inline std::size_t MM = __MM;

  static constexpr inline std::size_t NN = __NN;

  static constexpr inline std::size_t LAG1OVER =
      melg64::melg_base<__NN, __MM, __MATRIX_A, __P, __LAG1, __SHIFT1,
                        __MASK1>::NN -
      melg64::melg_base<__NN, __MM, __MATRIX_A, __P, __LAG1, __SHIFT1,
                        __MASK1>::LAG1;

  static constexpr inline int P = __P;

  static constexpr inline int SHIFT1 = __SHIFT1;

  static constexpr inline int W = 64;

  static constexpr inline melg64::result_type mask_u =
      (~static_cast<melg64::result_type>(0))
      << (melg64::melg_base<__NN, __MM, __MATRIX_A, __P, __LAG1, __SHIFT1,
                            __MASK1>::W -
          melg64::melg_base<__NN, __MM, __MATRIX_A, __P, __LAG1, __SHIFT1,
                            __MASK1>::P);

  static constexpr inline melg64::result_type mask_l =
      ~melg64::melg_base<__NN, __MM, __MATRIX_A, __P, __LAG1, __SHIFT1,
                         __MASK1>::mask_u;

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

    this->i_ = static_cast<std::size_t>(1);

    for (; this->i_ < this->NN; this->i_++) {
      this->state_[this->i_] =
          multiplier * melg_base::mat3pos(62, this->state_[this->i_ - 1]) +
          this->i_;
    }

    this->lung_ =
        multiplier * melg_base::mat3pos(62, this->state_[this->i_ - 1]) +
        this->i_;
  }

  static constexpr melg64::result_type mat3neg(
      const int t, const melg64::result_type v) noexcept {
    return v ^ (v << t);
  }

  static constexpr melg64::result_type mat3pos(
      const int t, const melg64::result_type v) noexcept {
    return v ^ (v >> t);
  }

  constexpr melg64::result_type next_lung(const melg64::result_type x,
                                          const std::size_t i) noexcept {
    return (x >> 1) ^ this->mag01[x & static_cast<melg64::result_type>(1)] ^
           this->state_[i] ^ melg_base::mat3neg(37, this->lung_);
  }

  constexpr melg64::result_type next_x() noexcept {
    return this->next_x(this->i_, this->i_ + static_cast<std::size_t>(1));
  }

  constexpr melg64::result_type next_x(const std::size_t i_u,
                                       const std::size_t i_l) noexcept {
    return (this->state_[i_u] & this->mask_u) |
           (this->state_[i_l] & this->mask_l);
  }
};

using melg607 =
    melg_base<9, 5, 0x81f1fd68012348bcUL, 31, 3, 30, 0x66edc62a6bf8c826UL>;

using melg1279 =
    melg_base<19, 7, 0x1afefd1526d3952bUL, 63, 5, 6, 0x3a23d78e8fb5e349UL>;

using melg2281 =
    melg_base<35, 17, 0x7cbe23ebca8a6d36UL, 41, 6, 6, 0xe4e2242b6e15aebeUL>;

using melg4253 =
    melg_base<66, 29, 0xfac1e8c56471d722UL, 29, 9, 5, 0xcb67b0c18fe14f4dUL>;

using melg11213 =
    melg_base<175, 45, 0xddbcd6e525e1c757UL, 13, 4, 5, 0xbd2d1251e589593fUL>;

using melg19937 =
    melg_base<311, 81, 0x5c32e06df730fc42UL, 33, 19, 16, 0x6aede6fd97b338ecUL>;

using melg44497 =
    melg_base<695, 373, 0x4fa9ca36f293c9a9UL, 17, 95, 30, 0x6fbbee29aaefd91UL>;

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
