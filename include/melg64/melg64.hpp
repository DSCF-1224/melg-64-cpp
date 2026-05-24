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
#define MELG64_H_

#include <algorithm>
// https://cppreference.com/cpp/header/algorithm
// https://cpprefjp.github.io/reference/algorithm.html

#include <array>
// https://cppreference.com/cpp/header/array
// https://cpprefjp.github.io/reference/array.html

#include <cassert>
// https://cppreference.com/cpp/header/cassert
// https://cpprefjp.github.io/reference/cassert.html

#include <concepts>
// https://cppreference.com/cpp/header/concepts
// https://cpprefjp.github.io/reference/concepts.html

#include <cstddef>
// https://cppreference.com/cpp/header/cstddef
// https://cppreference.com/cpp/types/ptrdiff_t
// https://cpprefjp.github.io/reference/cstddef.html
// https://cpprefjp.github.io/reference/cstddef/ptrdiff_t.html

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

#include <span>
// https://cppreference.com/cpp/header/span
// https://cpprefjp.github.io/reference/span.html

namespace melg64 {

using result_type = std::uint_fast64_t;

template <std::size_t __NN, std::size_t __MM, melg64::result_type __MatrixA,
          int __P, std::ptrdiff_t __Lag1, int __Shift1,
          melg64::result_type __Mask1, int __ShiftLungPos, int __ShiftLungNeg>
class melg_base {
 public:
  explicit melg_base() { this->seed(); }

  explicit melg_base(const melg64::result_type s) { this->seed(s); }

  explicit melg_base(std::span<const melg64::result_type> init_key) {
    this->seed(init_key);
  }

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

  melg64::result_type operator()() { return (this->*next_)(); }

  // Additions

  friend bool operator==(const melg_base& lhs, const melg_base& rhs) noexcept {
    return (lhs.i_ == rhs.i_) &&
           std::equal(lhs.state_, lhs.state_ + __NN, rhs.state_) &&
           (lhs.lung_ == rhs.lung_) && (lhs.next_ == rhs.next_);
  }

  constexpr void seed(const melg64::result_type s = default_seed) {
    this->initialize_member_state(s);
    this->initialize_member_i();
    this->initialize_member_next();
  }

  constexpr void seed(std::span<const melg64::result_type> init_key) {
    assert(!init_key.empty());

    this->initialize_member_state(this->default_seed);
    this->initialize_member_state(init_key);
    this->initialize_member_i();
    this->initialize_member_next();
  }

 private:
  using FuncPtr = melg64::result_type (melg_base::*)() noexcept;

  static constexpr inline std::ptrdiff_t Lag1 = __Lag1;

  static constexpr inline melg64::result_type mag01[2] = {
      static_cast<melg64::result_type>(0), __MatrixA};

  static constexpr inline melg64::result_type Mask1 = __Mask1;

  static constexpr inline std::size_t MM = __MM;

  static constexpr inline std::size_t NN = __NN;

  static constexpr inline std::ptrdiff_t Lag1Over =
      static_cast<std::ptrdiff_t>(__NN) - __Lag1;

  static constexpr inline int P = __P;

  static constexpr inline int ShiftLungNeg = __ShiftLungNeg;

  static constexpr inline int ShiftLungPos = __ShiftLungPos;

  static constexpr inline int Shift1 = __Shift1;

  static constexpr inline int W = 64;

  static constexpr inline melg64::result_type MaskU =
      (~static_cast<melg64::result_type>(0)) << (W - __P);

  static constexpr inline melg64::result_type MaskL = ~MaskU;

  std::size_t i_;

  melg64::result_type state_[NN];

  /**
   * @brief extra state variable
   */
  melg64::result_type lung_;

  FuncPtr next_;

  struct ZeroStateTag final {};

  explicit melg_base(const melg_base& other, ZeroStateTag) noexcept {
    this->lung_ = static_cast<melg64::result_type>(0);

    for (std::size_t i = 0; i < this->NN; i++) this->state_[i] = this->lung_;

    this->i_ = other.i_;

    this->next_ = other.next_;
  }

  void add(melg_base& other) noexcept {
    /* adds the lung */

    other.lung_ ^= this->lung_;

    /* adds the states */

    const std::size_t n1 = other.i_;
    const std::size_t n2 = this->i_;

    if (n1 <= n2) {
      const std::size_t diff2 = n2 - n1;
      const std::size_t diff1 = this->NN - diff2;

      std::size_t i = n1;

      for (; i < diff1; i++) other.state_[i] ^= this->state_[i + diff2];

      for (; i < this->NN; i++) other.state_[i] ^= this->state_[i - diff1];

      for (i = 0; i < n1; i++) other.state_[i] ^= this->state_[i + diff2];
    } else {
      const std::size_t diff2 = n1 - n2;
      const std::size_t diff1 = this->NN - diff2;

      std::size_t i = n1;

      for (; i < this->NN; i++) other.state_[i] ^= this->state_[i - diff2];

      for (i = 0; i < diff2; i++) other.state_[i] ^= this->state_[i + diff1];

      for (; i < n1; i++) other.state_[i] ^= this->state_[i - diff2];
    }
  }

  constexpr void initialize_member_i(void) noexcept {
    this->i_ = static_cast<std::size_t>(0);
  }

  constexpr void initialize_member_next() noexcept {
    this->next_ = &melg_base::next_case1;
  }

  constexpr void initialize_member_state(const melg64::result_type seed) {
    constexpr melg64::result_type multiplier =
        static_cast<melg64::result_type>(6364136223846793005UL);

    this->state_[0] = seed;

    this->i_ = static_cast<std::size_t>(1);

    for (; this->i_ < this->NN; this->i_++) {
      this->state_[this->i_] =
          multiplier * this->mat3pos(62, this->state_[this->i_ - 1]) + this->i_;
    }

    this->lung_ =
        multiplier * this->mat3pos(62, this->state_[this->i_ - 1]) + this->i_;
  }

  constexpr void initialize_member_state(
      std::span<const melg64::result_type> init_key) noexcept {
    constexpr melg64::result_type multiplier1 =
        static_cast<melg64::result_type>(3935559000370003845ULL);

    constexpr melg64::result_type multiplier2 =
        static_cast<melg64::result_type>(2862933555777941757ULL);

    const std::size_t initial_i = static_cast<std::size_t>(1);
    const std::size_t initial_j = static_cast<std::size_t>(0);

    const std::size_t key_length = (std::size_t)init_key.size();

    std::size_t i = initial_i;
    std::size_t j = initial_j;
    std::size_t k = std::max(this->NN, key_length);

    for (; k; k--) {
      this->state_[i] =
          (this->state_[i] ^
           (this->mat3pos(62, this->state_[i - 1]) * multiplier1)) +
          init_key[j] + static_cast<melg64::result_type>(j); /* non linear */

      i++;
      j++;

      if (i >= this->NN) {
        this->state_[0] = this->state_[this->NN - 1];
        i = initial_i;
      }

      if (j >= key_length) j = initial_j;
    }

    for (k = this->NN - 1; k; k--) {
      this->state_[i] =
          (this->state_[i] ^
           (this->mat3pos(62, this->state_[i - 1]) * multiplier2)) -
          static_cast<melg64::result_type>(i); /* non linear */

      i++;

      if (i >= this->NN) {
        this->state_[0] = this->state_[this->NN - 1];
        i = initial_i;
      }
    }

    this->lung_ =
        (this->lung_ ^
         (this->mat3pos(62, this->state_[this->NN - 1]) * multiplier2)) -
        static_cast<melg64::result_type>(this->NN); /* non linear */

    this->state_[0] =
        (this->state_[0] |
         (static_cast<melg64::result_type>(1)
          << 63)); /* MSB is 1; assuring non-zero initial array. Corrected. */
  }

  void jump(const char* jump_string) noexcept {
    melg_base melg_init(*this, melg_base::ZeroStateTag{});

    const std::size_t total_bits = this->NN * this->W + this->P;

    const std::size_t total_chars = static_cast<std::size_t>(
        std::ceil(static_cast<double>(total_bits) / 4));

    for (std::size_t i = 0; i < total_chars; i++) {
      char bits = jump_string[i];

      if (('a' <= bits) && (bits <= 'f')) {
        bits -= static_cast<char>('a' - 10);
      } else {
        bits -= '0';
      }

      bits &= static_cast<char>(0x0f);

      char mask = static_cast<char>(0x08);

      for (int j = 0; j < 4; j++) {
        if ((bits & mask) != static_cast<char>(0)) this->add(melg_init);
        (*this)();
        mask >>= 1;
      }
    }

    this->lung_ = melg_init.lung_;

    this->i_ = melg_init.i_;

    this->next_ = melg_init.next_;

    for (std::size_t k = 0; k < this->NN; k++) {
      this->state_[k] = melg_init.state_[k];
    }
  }

  static constexpr melg64::result_type mat3neg(
      const int t, const melg64::result_type v) noexcept {
    return v ^ (v << t);
  }

  static constexpr melg64::result_type mat3pos(
      const int t, const melg64::result_type v) noexcept {
    return v ^ (v >> t);
  }

  constexpr melg64::result_type next_case1() noexcept {
    melg64::result_type x = next_x_1st();

    this->lung_ = this->next_lung(x, this->i_ + this->MM);

    this->state_[this->i_] = this->next_state(x);

    x = this->next_x_2nd();

    x = this->next_x_3rd(x, this->Lag1);

    ++this->i_;

    if (this->i_ == this->NN - this->MM) {
      this->next_ = &melg_base::next_case2;
    }

    return x;
  }

  constexpr melg64::result_type next_case2() noexcept {
    melg64::result_type x = this->next_x_1st();

    this->lung_ = this->next_lung(x, this->i_ + this->MM - this->NN);

    this->state_[this->i_] = this->next_state(x);

    x = this->next_x_2nd();

    x = this->next_x_3rd(x, this->Lag1);

    ++this->i_;

    if (this->i_ == static_cast<std::size_t>(this->Lag1Over)) {
      this->next_ = &melg_base::next_case3;
    }

    return x;
  }

  constexpr melg64::result_type next_case3() noexcept {
    melg64::result_type x = this->next_x_1st();

    this->lung_ = this->next_lung(x, this->i_ + this->MM - this->NN);

    this->state_[this->i_] = this->next_state(x);

    x = this->next_x_2nd();

    x = this->next_x_3rd(x, -this->Lag1Over);

    ++this->i_;

    if (this->i_ == this->NN - static_cast<std::size_t>(1)) {
      this->next_ = &melg_base::next_case4;
    }

    return x;
  }

  constexpr melg64::result_type next_case4() noexcept {
    melg64::result_type x =
        this->next_x_1st(this->NN - static_cast<std::size_t>(1), 0);

    this->lung_ = this->next_lung(x, this->MM - static_cast<std::size_t>(1));

    this->state_[this->i_] = this->next_state(x);

    x = this->next_x_2nd();

    x = this->next_x_3rd(x, -this->Lag1Over);

    this->initialize_member_i();

    this->initialize_member_next();

    return x;
  }

  constexpr melg64::result_type next_lung(const melg64::result_type x,
                                          const std::size_t i) noexcept {
    return (x >> 1) ^ this->mag01[x & static_cast<melg64::result_type>(1)] ^
           this->state_[i] ^ this->mat3neg(this->ShiftLungNeg, this->lung_);
  }

  constexpr melg64::result_type next_state(
      const melg64::result_type x) noexcept {
    return x ^ this->mat3pos(this->ShiftLungPos, this->lung_);
  }

  constexpr melg64::result_type next_x_1st() noexcept {
    return this->next_x_1st(this->i_, this->i_ + static_cast<std::size_t>(1));
  }

  constexpr melg64::result_type next_x_1st(const std::size_t i_u,
                                           const std::size_t i_l) noexcept {
    return (this->state_[i_u] & this->MaskU) |
           (this->state_[i_l] & this->MaskL);
  }

  constexpr melg64::result_type next_x_2nd() noexcept {
    return this->mat3neg(this->Shift1, this->state_[this->i_]);
  }

  constexpr melg64::result_type next_x_3rd(const melg64::result_type x_2nd,
                                           const std::ptrdiff_t lag1) noexcept {
    return x_2nd ^ (this->state_[static_cast<std::ptrdiff_t>(this->i_) + lag1] &
                    this->Mask1);
  }
};

using melg607 = melg_base<9, 5, 0x81f1fd68012348bcUL, 31, 3, 30,
                          0x66edc62a6bf8c826UL, 35, 13>;

using melg1279 = melg_base<19, 7, 0x1afefd1526d3952bUL, 63, 5, 6,
                           0x3a23d78e8fb5e349UL, 37, 22>;

using melg2281 = melg_base<35, 17, 0x7cbe23ebca8a6d36UL, 41, 6, 6,
                           0xe4e2242b6e15aebeUL, 21, 36>;

using melg4253 = melg_base<66, 29, 0xfac1e8c56471d722UL, 29, 9, 5,
                           0xcb67b0c18fe14f4dUL, 20, 30>;

using melg11213 = melg_base<175, 45, 0xddbcd6e525e1c757UL, 13, 4, 5,
                            0xbd2d1251e589593fUL, 13, 33>;

using melg19937 = melg_base<311, 81, 0x5c32e06df730fc42UL, 33, 19, 16,
                            0x6aede6fd97b338ecUL, 33, 23>;

using melg44497 = melg_base<695, 373, 0x4fa9ca36f293c9a9UL, 17, 95, 6,
                            0x6fbbee29aaefd91UL, 14, 37>;

}  // namespace melg64

static_assert(std::unsigned_integral<melg64::result_type>);

static_assert(sizeof(melg64::result_type) >= 8,
              "`result_type` must be at least 64-bit");

static_assert(std::uniform_random_bit_generator<melg64::melg607>);
static_assert(std::uniform_random_bit_generator<melg64::melg1279>);
static_assert(std::uniform_random_bit_generator<melg64::melg2281>);
static_assert(std::uniform_random_bit_generator<melg64::melg4253>);
static_assert(std::uniform_random_bit_generator<melg64::melg11213>);
static_assert(std::uniform_random_bit_generator<melg64::melg19937>);
static_assert(std::uniform_random_bit_generator<melg64::melg44497>);

#endif /* MELG64_H_ */
