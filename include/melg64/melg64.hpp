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
// https://cppreference.com/cpp/algorithm/equal
// https://cppreference.com/cpp/algorithm/max
// https://cpprefjp.github.io/reference/algorithm.html
// https://cpprefjp.github.io/reference/algorithm/equal.html
// https://cpprefjp.github.io/reference/algorithm/max.html

#include <cassert>
// https://cppreference.com/cpp/header/cassert
// https://cpprefjp.github.io/reference/cassert.html

#include <concepts>
// https://cppreference.com/cpp/header/concepts
// https://cpprefjp.github.io/reference/concepts.html

#include <cstddef>
// https://cppreference.com/cpp/header/cstddef
// https://cppreference.com/cpp/types/ptrdiff_t
// https://cppreference.com/cpp/types/size_t
// https://cpprefjp.github.io/reference/cstddef.html
// https://cpprefjp.github.io/reference/cstddef/ptrdiff_t.html
// https://cpprefjp.github.io/reference/cstddef/size_t.html

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

template <std::uniform_random_bit_generator URBG>
struct jump_string;

template <std::size_t NN_, std::size_t MM_, melg64::result_type MatrixA_,
          int P_, std::ptrdiff_t Lag1_, int Shift1_, melg64::result_type Mask1_,
          int ShiftLungPos_, int ShiftLungNeg_>
class melg_base {
 public:
  /**
   * @brief Constructs the engine with the default seed value.
   */
  melg_base() { this->seed(); }

  /**
   * @brief Constructs the engine with a single seed value.
   * @param s The seed value used to initialize the state.
   */
  explicit melg_base(const melg64::result_type s) { this->seed(s); }

  /**
   * @brief Constructs the engine with an array seed.
   * @param init_key The key array used to initialize the state.
   * @attention !init_key.empty()
   */
  explicit melg_base(std::span<const melg64::result_type> init_key) {
    this->seed(init_key);
  }

  /**
   * @brief The default seed value used when no seed is specified.
   */
  static constexpr melg64::result_type default_seed =
      static_cast<melg64::result_type>(19650218UL);

  // std::uniform_random_bit_generator Requirements

  using result_type = melg64::result_type;

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
   * @brief Generates a pseudo-random value.
   * @return A pseudo-random value in [min(), max()].
   */
  melg64::result_type operator()() noexcept {
    switch (this->selector_) {
      case 1:
        return this->next_case1();
      case 2:
        return this->next_case2();
      case 3:
        return this->next_case3();
      default:
        return this->next_case4();
    }
  }

  // Additions

  /**
   * @brief Compares two engines for equality.
   * @return true if both engines would generate the same sequence.
   */
  friend bool operator==(const melg_base& lhs, const melg_base& rhs) noexcept {
    return (lhs.i_ == rhs.i_) &&
           std::equal(lhs.state_, lhs.state_ + NN_, rhs.state_) &&
           (lhs.lung_ == rhs.lung_) && (lhs.selector_ == rhs.selector_);
  }

  /**
   * @brief Advances the engine's state by 2^256 steps.
   * @note Equivalent to calling operator() 2^256 times.
   *       Useful for generating disjoint sequences in parallel computations.
   */
  void jump() noexcept {
    static_assert(
        requires { jump_string<melg_base>::value; },
        "`jump_string` not defined for this variant");

    this->jump_impl(jump_string<melg_base>::value);
  }

  /**
   * @brief Sets the current state of the engine with a single seed value.
   * @param s The seed value to use to set the state
   */
  constexpr void seed(const melg64::result_type s = default_seed) {
    this->initialize_member_state(s);
    this->initialize_member_i();
    this->initialize_member_selector();
  }

  /**
   * @brief Sets the current state of the engine with an array seed.
   * @param init_key The array seed to use to set the state
   * @attention !init_key.empty()
   */
  constexpr void seed(std::span<const melg64::result_type> init_key) {
    assert(!init_key.empty());

    this->initialize_member_state(this->default_seed);
    this->initialize_member_state(init_key);
    this->initialize_member_i();
    this->initialize_member_selector();
  }

 private:
  using FuncPtr = melg64::result_type (melg_base::*)() noexcept;

  static constexpr std::ptrdiff_t Lag1 = Lag1_;

  static constexpr melg64::result_type mag01[2] = {
      static_cast<melg64::result_type>(0), MatrixA_};

  static constexpr melg64::result_type Mask1 = Mask1_;

  static constexpr std::size_t MM = MM_;

  static constexpr std::size_t NN = NN_;

  static constexpr std::ptrdiff_t Lag1Over =
      static_cast<std::ptrdiff_t>(NN_) - Lag1_;

  static constexpr int P = P_;

  static constexpr int ShiftLungNeg = ShiftLungNeg_;

  static constexpr int ShiftLungPos = ShiftLungPos_;

  static constexpr int Shift1 = Shift1_;

  static constexpr int W = 64;

  static constexpr melg64::result_type MaskU =
      (~static_cast<melg64::result_type>(0)) << (W - P_);

  static constexpr melg64::result_type MaskL = ~MaskU;

  std::size_t i_;

  melg64::result_type state_[NN];

  /**
   * @brief extra state variable
   */
  melg64::result_type lung_;

  int selector_;

  struct ZeroStateTag final {};

  explicit melg_base(const melg_base& other, ZeroStateTag) noexcept {
    this->lung_ = static_cast<melg64::result_type>(0);

    for (std::size_t i = 0; i < this->NN; i++) this->state_[i] = this->lung_;

    this->i_ = other.i_;

    this->selector_ = other.selector_;
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

  constexpr void initialize_member_selector() noexcept { this->selector_ = 1; }

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

  void jump_impl(const char* jump_string) noexcept {
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

    this->selector_ = melg_init.selector_;

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
      ++this->selector_;
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
      ++this->selector_;
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
      ++this->selector_;
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

    this->initialize_member_selector();

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

template <>
struct jump_string<melg607> {
  static constexpr const char* value =
      "f3d27aef5c025caca71e8dfb38d8e7ce5fe0d46c04317c6f50"
      "ef41c5edce6ebf48fe2929dd0ca41af901d536b52ae616662b"
      "620bad0a18060e54c127d729bdcb439f7ee398bec8e7195562"
      "9c";
};

template <>
struct jump_string<melg1279> {
  static constexpr const char* value =
      "a4704d47efb161016e3736c80e933688017732e3ffc4115893"
      "8838ba22bb5cddf444d6f3fb8f3431c350ef813cceb90a9587"
      "b8e1626e74dc53831fba639564f313238548597b13bc13679e"
      "172cf95e9fabac836d6888253c34e4ac182c6779be5414e2cb"
      "1933412fcbdc47a055d72c339f5033276d8cc5b491ec343bbe"
      "7f5467cd6ed8e33b8f1305b10e3b134e67c62358665d196e5c"
      "2030a9e45ae42eab5e0c";
};

template <>
struct jump_string<melg2281> {
  static constexpr const char* value =
      "153f3f5f58ab21e2b7e825fdc3cf74144f37d5320d6d4a08d4"
      "5b84ceb30294b6f66be04d2b9a7bd2fe0ffe28dfc60c814e82"
      "c4f85543a992fb7abf20f2f45c4b9e10729797ee8c34624102"
      "b21adc05b2abaf1e08bd353b30d2ee3b889f4df1209245d8f5"
      "4c836ee63466f0ed7bbf5816c6d3b36c9676b8a9d48f82a60a"
      "87d7d40a5da53a7fcf46ee5f3052bb8010509c9a550d29867c"
      "0f8d0b65ac69c69889d72ef9f7d782dacdb6d849a54d67c5d1"
      "98468a02b28eabac4fa905fb06a1c2cd8def5e9ee05da25d92"
      "be43269cddcd54a96543292fb854cd62a1d45c417f8666ef7c"
      "fa5404456991aec230fe92c6eb513151d9810de985906e49f6"
      "245bbcdcf257700469db91830d7e08dab027f5bb294962cf6b"
      "bb3b53f1c22932113a870";
};

template <>
struct jump_string<melg4253> {
  static constexpr const char* value =
      "514609396aa32e1815afd614eabdd3ebacb4868f08cfed4ca1"
      "e27c40ed5a24db338fe372795db756f0f632ce67327a5e61e5"
      "53e9248920f860bf759719e5db8ace1d5334763fa5df0e92dc"
      "9e78719aae25aa0e8125b0a63fc035b9605c185a4fe35cc18f"
      "98210fb398dc6cd68932a6c4dead9efd6f410086ccbe8d2518"
      "9be700ed70bd07af780e7cbda0172647d929221aec90cd5bc7"
      "1d52b673c34edf12ab6fa5b72cb466b514dec1695e3aafbc15"
      "6e1a4c7d289d7644359e108ccad0247e120f3ca0d7d5007776"
      "f2df463f383eaa3abe97e4248764e79e8219ac22b00c622376"
      "d0d17dcb3d280de5e87c3b0b826a65c36c84704026ef8351df"
      "3e7f428d113311ae397d20fe518709867ae8f076ae58cf2498"
      "945fa9fc5dfa12d6db79078d3ad42c07655feb5a7846af5d6d"
      "1422db5ae9dcc999418ac24a1f4e4c2145fa7a7c74631de210"
      "6b284f0f26377cca29a1740104cdb723b8c907d50204da74d2"
      "d3ebe9fa9eed13e21ed507151567b864798ac67aa55dec472f"
      "2907042795f242448c0b9772d51b18fd7ce9b2eed2effbd069"
      "417d2d1ea1b14d2b5753d3a11295aa1de6b0d9e7172646c86f"
      "610133672dd7a1014e2916c5dbdff1040034e07d52e90edb36"
      "59e7b612a1c45d49280072c8da8a0f5e5497d8eebde67d6a9a"
      "1b29375720036d84245ff9b96c670c555610f35ed15889e97a"
      "cc8a7812be1fd09440714e353d1c197a9addf30acc7a0bc90f"
      "b1e354125eb388";
};

template <>
struct jump_string<melg11213> {
  static constexpr const char* value =
      "f455de2f54520b2dd8ce35d97fd8b50e4ef4f8776263cf2a26"
      "f392b0e71ad26509d8d20ada6ff2b026ca14e21cc91597f6af"
      "280a5e22ecde9f9e6653fedcba04c3ece760f941994129db87"
      "c28701d351c7d54c951a38c11490cea33913acd7e0d5960300"
      "69e553711ebd8ac376a32a080863759449d00326bfc085951a"
      "2014ed68db92fb6a0db234c6175dd7e38fc03352e91d9988a5"
      "6734941d00e65311c51f6358653537be02609619a45c1c3d12"
      "c257a1d559d4b485df547b60aff026ac32dcfa01f1e2d2105a"
      "d68e3a268c67594f8b90bee09fe3df899ce7c2f02eb05070d3"
      "707f50d2f6f8bd2864833733fd6505bd2d08deefccea8725c4"
      "7fd4febd22882362955bf5bd8a604118bc1ce86a8f9d0a76bc"
      "6f709cfb024f4d2c68a01777d344a10316999c02093f73e14f"
      "5cf573e4242e09ff6c154a77dc7796f812ee713886219caae3"
      "f3db710d37b215b79a49fbc6339eedcc63785fa5f96fbf0352"
      "13d4455545447e708d555f6b060c0fd27d713a0285bb7b1565"
      "6cc5bde689b6a61f5ab0140b4a7d2ed3b44ee302f9e21e747c"
      "9ed7d1d5005e72f00dc6a5e6264d44deff7fd1ed39fa33dde6"
      "11adf01d41e1241b9eed738b88c3280523126b6c5c16679cbf"
      "9aaf27e035bbc52449d4d7e2f129b18924549b731c7efd1c41"
      "20898ed1fe1f005db1160a6918c8b22823385aea6961f18487"
      "e3bfb3ec478ac6c9c5fe8b89f590cd59ddb4dec62ee27bc32d"
      "534566c4d4e9c8eb6989a74723b11baf1524db1ed389e8f245"
      "e97fd9322674e177e5e34eef5254aafac857351aa2a352f0d2"
      "128d4f97b6803e02031e25b5f51ba3f7cb1d789614d01d3a82"
      "7067c24deb627f8a84eee83dcff9ce5e5886981388ea4a8145"
      "2e6b68e05227585898f16ce6a163cfca9db2964ade5ffc7754"
      "88e48e9f1ea25afcdaf40745087fa358bacb25070a1a23b0ff"
      "7f67670327fd511a897ff2aa995c51f4218217d604495aed21"
      "3a99b36560444269159c1510083af95f2660abf69e311bed3d"
      "fd37177aac8784d12f1654d49197ba58b598f8bd0aea9711c6"
      "8bffded44acb765090baf2744e43fba03f80ed667e3880b2a4"
      "fe0a56b4c84b2ecdc76f2150615cf54ad6469e4fcf10ab3f20"
      "be5080409cc4325d67a91f535b2ed5c980f27142904ef64f5d"
      "e5bba1e5ef812c6802dc8671da7a60cd74c74277a19c01c917"
      "c275594677bee39ba5afbbf5ebfd7ac0c5cde0cf1c4df295d6"
      "ae684dc704937f930d3200f163d54760bdd961e96ffc64aae2"
      "d05f357362b29e0dda3e808366808aca5ff9c0391b7b00b9fc"
      "f923d7b5ebf3d876fb1ea6058febf4c65c5f925c6a8b7a2af3"
      "f32988838dbf961486f1805b68f7b53c74dabb57c89f0d4460"
      "6897da58f9967d79270678b7e0bebd9bbaa058152f17c7c0ee"
      "5fbf16ad091f19b744f11a980084b3a585682a413fe1b30aa5"
      "0f836e60c647fa9d27628e7a5abe87dbda4cd4c4ecc165cafb"
      "307ad89d35655188d77f1d5ef339a1b84cef2c5991a3078900"
      "dbeeb76b0516d27f22e98a4c0fb9282535d2a8afa7bd0b7ddc"
      "9c14b007eb5dc9bed6ef7fe875758977750d9f1420110c3804"
      "645c4524abe712a40c426d466096ca8ad26232c4441f1d2091"
      "3e1be38e0fbad2c67304d39ce9a5e130b06896c40348a52b32"
      "be85dcae102dd2b631301decf98b436a52891fac17bdca7e7a"
      "0f9e7a685fb8c1c58f2095c7e44c67cbe61ad1015947043d97"
      "bb9d9d40d5d54eba0f14383f6a16fce87e1d0719528c25c71b"
      "441672c77b0a4dfb081176ed7add6b804052de243b08b9ab30"
      "517c1ee135c7b3690b7805b0d2664cbba443d0bf61ca0b276b"
      "332b641579ca19de873a074e642eeb5cdb4a202bed3e0cb8fd"
      "1bfc313f5013fdb325e217790e418acb9da12088efb02ee740"
      "708c49f4136c8f31c60998c756c8af8537426dd25ae152e655"
      "27c66b475477fbabe007c9e65b91acceb9227d3baf24dccd8c"
      "18d0";
};

template <>
struct jump_string<melg19937> {
  static constexpr const char* value =
      "1510de5f1aeb1b349b7d2f3dc278bf1e6358d09c083c53b2b5"
      "2b0b37aa42ec96ae92d9199e5ddb4f8f19419a1ae8d41d208c"
      "c209439db14c17bc032c1aa482b589174bb3ac3964a128c742"
      "017ff511a9ddd720f397969f0c4dc862608725d5465dd0d257"
      "99d29ff579515657f3b7f58f5f6090d3c2c283b9e1cc517b48"
      "d4df4f03db955624557939ba23ff0b68b195a7a7413dcb3029"
      "25711acc4fbc5554193ddcf43bfd9deeda0e3a684770ef6b11"
      "b8129f937e0c41e8c7c435bb76c6ca0518d6cd8809410c33a5"
      "f5f39573f7ed9479abe9a5ee7bf09e189b1737f6fe53897026"
      "d792327de7e2c9ca050fa66f23eab9a0a83b67a9e6d54d70ce"
      "46664dbc4af7cee88756fc50f16b841b76167c66613ef43b00"
      "b775aeed0e260fde67da03f6051ba11dbfa2070447f3aba151"
      "e001404a11d3049e53f177ee4c275cffcf4c6e5c7b8a1e8db0"
      "86731abb01ea50ec8440bc45fdd3c23679a68b29b2457d0013"
      "878d8a7f1dccc595f99e656b64da2715a392eb68a517989be2"
      "4c663dcbfb663ff38c567fa6b5fe8bdccbd30163524a9a1d63"
      "cf609eb93a1fe3cca5e1220bd05e4dcb611a459d6ee70bbf57"
      "86d6fb887aea96e70e78af7f50dcbc638664ac28efcab6356d"
      "ed959bb79355c5bc5e189a20bb8f64e5fcb444c2f29c57fce7"
      "a70208115da1b8a663c8062cbc98e353526b1d72371c07fb0c"
      "ad50a923eef2c5c865d733be91978e1279cc45ea20f534e428"
      "422f72c30957e7fab79da909526d097b4a3a790c2b3cae28ef"
      "52e5eb4302858110e1bcc31187bdbf79012e770ff95126a7a0"
      "4b4059e2a9f9f885a6af3d5d067148e05bdd01bdc8f7a33b47"
      "5631f89a08e92e61a25618846b55a2f42ab42c56ce3d3948fd"
      "f515b90b344f726bfe8543a93367cd5d95b08d4da0bcc7b2fc"
      "65384a51eb16766ee2ee3bdf82b6cf24c7a81e826d2e9f81e8"
      "1917ead9c3ca2b0ea0a2395cf4804080dd0cbf4698e412b7a2"
      "49ddc89bc939e34857437be5fc1586f932a0a10c48121eb5e8"
      "3a1d4e4bd682d9674d6d42f8ec190dada2ba9c4c0c25392b1c"
      "fc32916c9f7dd5978badc53796d2c2843880adfaff7d83b73c"
      "5959b9a7424715d2f7a47e1c0363c7d3f60c332c8bb39b8656"
      "08c1035c2773f53a0edc2582182a5cffaa5acd15820daeff16"
      "58c64ac4b579f8134fd1db297c1d4d4dd03b4f063a293a2cbd"
      "a3aaf381e6cf54a0cd949e5ed2473852484566db89de18654d"
      "8efa020ed963c9d26dbba50a3de5f0c3b6e72b477c8f26284d"
      "cf561c3df5780cef6197039cc076391022a0d57845e992e3b5"
      "2189c95e92172461838b14f014f452ab24460be82113d41f31"
      "47e210c03f8430b223836d1efe5ef96bf56708dbad033d57fa"
      "74beb1314c1abf1b328b4145c359bc4b6befc94c6bec8762f5"
      "feaa4f14f309e5e51415479d1f16821528b707599eb530a898"
      "6b751ccce0d17055894116cd032af55860af016dff76fa14ce"
      "b606c4b277f5968f897d91b544db7cf0de9fb237d599000751"
      "7e0aab7a73866d498e76f772006d3bf2387c552ba3d72e3a6a"
      "a324edeea5989a45b0468ec514127156141de06e22c78347d6"
      "dc48c07dd42b1a9c543deed9006daa8ae676dc328f7dbc5d90"
      "02d2f481f9cc4c7b9a433377bf61d0d75eae143ff8c7e7e0f0"
      "9a805ee12e187c02724a9c5e6789dd2a5300753bdfcc1c964c"
      "818d2a45e13e4ba89ea90fdd45b40a1b76079cbcbfc717162e"
      "b27d7a902f213646ed65e7f00e5fbc0cd74bb099e00ed350b4"
      "93225e88e5693d999244b8d0f1f9bbfad03e5223416fd790bc"
      "c6e047abd1523245c6a46d397f63b38ecebaf79234b53b9b02"
      "374cdf7bcaa9558043e1018eb14ec31b1fb56a7e6aa6730108"
      "12cf5abc0ed2ec1df75a615632f59968a92de6cc183c4c1555"
      "3fe5ca263cf3cffd1342e60975ac2de843f5b5a6314e382dd6"
      "a6887b87e29f9b31b0d7a2dc31e9f07212fa0c2e69db50d30b"
      "d676460a94a9822f5aaf5af01bc566136da7138ba69554577a"
      "2ef2f5d91051ec7ee3645a0df47bbea49e2a47c1279e3510e0"
      "8c89c9d5b20966125b582469b13d99308119423dab451f29b8"
      "b4f6ebeff94a06c74d9f6e040c269c39b1c5942cd96f812b35"
      "b047357ddb08863649a13cb38a4e10d047b8aa84a81870de3c"
      "d774a4b6174291bc3731437aefa7dbbf2af9c497dec0a90a36"
      "55395944fc6a0c3e46326a10d905fbd5cd90ccd46baac32cff"
      "4f6e48936de047e3eb24cf7e7e64ac7616ed8fe0ad751daee7"
      "bc8e09ab4447718355e92fbd583a3165466d722c4fb0f904d8"
      "65b77b99053db2709ae3c721b714ae8bbdac87fc0b81a5c5dd"
      "c2e042e3155801276efc14e508e5fff27ad21ff1c975657373"
      "20b1344df216188bb3872a28c11ecc1aabce8cdf9749b6bc67"
      "39628e3f35b531a32dac218196becb2945904b35079ce2bbd9"
      "7f811fb71c2fa1d9cc5ea65a9d88ee77ab2a52e48e8aaf4e4d"
      "91679618ffe441b8c319bf6c6589e118f3abd0f8c22fc930af"
      "64e1b0e4616c1f5f94c50ea240ea8cdd7d57f9b7ee11c3516f"
      "16115bc995e586f3483ca5be4bbf1c1fe4578934f77c03e307"
      "f6096854e9a93d28cd7331ce91371a2f50ae608d1f0348f8ce"
      "3ce48eaaf83f7195ea7b3fbcf4b331d4a2c7f21843b745164e"
      "4b71678b8ea41580feef7db43f090915ec7edae77eb058d37f"
      "a04571f4bad32d08d364301a7f0fc633fdfe3f9695f0edf8de"
      "2187dee171988c47da64da030fcbcfd8fc3b77a59943d46927"
      "c869e6065b237a0d9e32a72cf0e15ae969b0672a5f5835cdba"
      "88ce9173abe094d95ae7acee85e176fb826b9ffe01ca860f95"
      "06540e6f415a9c5ba8ad9a8dd306188fc1973dcd33f75c4b58"
      "f5d6a6df6a5ed88f4514690dee844b77c5fc6bb2090d5b6364"
      "fc31b0ec50e29cca44752024bc3270f553570ac196066eb1f0"
      "4e09be04b7301a915080ebeaea4c749c04f2d4cf79c5805d08"
      "beb34b966fbc5e153f80a00101883c93861bbee60c52470053"
      "546aeb57e487092b60884ab20f738f87c9ab6bca2a3370ffaf"
      "745ccbc44bae13befd29deacddb38d0124e02ef8aa656a87f7"
      "47e0deac35e7fe2f191ed119a6908a909222deffb028e5e12f"
      "ea7c3be122fb684ebf83f8adcba142affa7753e27370b493fe"
      "d258a4db5068042a9e4db38d160f388f4064dfd13b3bbfe95b"
      "cd6176ce99fef56573fc8141bc4a290202b2437df2886f2dcf"
      "b693d3110b78220a7007b695bfda744a356cbce15814d2eaf7"
      "1e322e9542d4933c7051e83f5a1636c72bda12822d803ca4da"
      "a66e5baa793271a6b301d1ec7a818a4b5ddca7d1141d830883"
      "cd1586b50b0cdee0f4d445752b2716b5cc44d8b2e1149b4ec4"
      "ca06f87fa7be9b4aad509804b64f3edebba10fc687f20d238a"
      "39f3b219c2e8f8f6f3533671843a521a457df1dbccc54b624b"
      "a0609fed10acfb9b3442bbf93f5689415d4243a06f53958e06"
      "f28b7b4e5d08ea178bc92eee27adb94f002b7d0bbc0da40075"
      "2421ab4edcce592d9996d2472b967043d20";
};

template <>
struct jump_string<melg44497> {
  static constexpr const char* value =
      "a31871495c2f64e9655938d42afec1430ebcc71124b13d5307"
      "90a3dee960d1ee0bb6cc00644378319e36de4ede4065a9f5e8"
      "eae02572ed6f250e22636e39ff9a27fc7090d590afc519f3ea"
      "8d3acf0596c3fef37c1076e40125cb16732423af58b3c45b92"
      "3d7ef14d0cab20f1f08f7e6c8d5fdbea98239a80ec69a429bc"
      "ba74d3268fbe79c67717fdd4d9a854f88ff882d6028cd228a6"
      "6908bf615a2a04c9d3f9c8dae5cf7e9e41f650fd610f45276c"
      "45607580b95041d9f8aa66fcf951d3dc7d5728d36eb186d875"
      "5e1cc29818496442eeb2d702f4efeb3a2c8be9dfdf996c0bb4"
      "5ff97e14f4fa3a9c53a08c452afc3bfcc41c5257957ad1c01f"
      "17cc80fd623d685bb8936f388266c0117f8a5a196488ebc18f"
      "1ad252556625a6f710757a1db096b281084a672e093f6c84cf"
      "29e3ba58044eed0c8374f8e09126b9080e10d8c6e33d66339b"
      "60ece9dc27df759e51ed311aebec77b83b47ac172919958163"
      "b863203fc60f018b9f36d227489bb2a8c46f3653a8143d08bc"
      "9a96a6acd387c1b9e4478b8ffdbcf551982746f2bf021c6c2c"
      "83662de7eccf542d6daf0d997f871e0b6745d31abbf219b036"
      "466d9412736c5181639452b9d5d6290b2416317e4125e5739d"
      "d1860d15109f1cc7bca0fcc048a3d0097ba1577138e100c112"
      "9692c77d7a09a01c07b9e15513d6679e615e481932fb4a0a17"
      "98b0236610fd0cf9caa571b6e0c180f68532d42384599eca39"
      "1aa75c9f3e8b9c46012ee53a0c766da2b9fec1703ac90975ae"
      "78f27497f9d01afc7f8de438c485e3dc316acb83999ef522f7"
      "dcd99d48a0450d443aa4b305258fb1c8859de56b017eb587ad"
      "f1b0642186179310dbc949c660ddb5560beacad9a28e550907"
      "ca4850b0db0b1521c2ea13d4ff184d387d51d19b82210815a5"
      "824a9bedb8b67804ba097d2bfe4012cec291b48260c15c95ec"
      "969b83546c0dcc4b2b435f1c7b6fd1b03ad258d34c5b22c30d"
      "0ba88144f844d9928258b784c416f9594acdd17d71f8f07164"
      "dc0aa457e751b35813ab6a83abd114c1f938d5b4f839354248"
      "6e665e9946022e781648ea82cc08133ad2e71578dbc1150e52"
      "d0338bbf4c7297559450c16c6420995789aca0f16cd2eed150"
      "5a5896f9ed840e9646e36ac47019ccf297697d2d26aa186aa9"
      "fa09dbd106aa1796fad677db1360a784d05d13c06b4cfe445a"
      "6c0aa463bccf73be5f6f609e9876d940230938423f7abc6514"
      "d51cb17e645e40a4f685680651eab70a8c17af64e43c6486db"
      "9e411dc5f912c44e6bbaeecadc39e4f805f177b642e6a1e029"
      "e7a15acaa2f5d71dc2aa1d9775b6c51ef4293353a10145d0d3"
      "f2d8485863484f0a1a48c28f7d56ca311343fb2e18550fd4e6"
      "7c77da467784acfff8652f959b481a2a4918b8aabf9c0176ee"
      "14e2c6b7279b810a1696050365bbf1f7cd023bb98caad2d0f6"
      "9cc3ac04d1921c1007eee16d6d9188352580f2f8dee387aecc"
      "1abadd739d932ef5b945ee57532aeff57bdc1e492d261ef2bf"
      "7c467c142eee905c8dbe5f452f51839e66fd793575c3b4fe74"
      "8b205662d4e2aaa63f857f8e42e80e79b64bf86dbbf818aad6"
      "6b3c8fc1bc404a83eb155a32a4a16049795f46433801d59a78"
      "f3a137aa12c698ee2fa24331fd10e990bde99e077fdb9d4e97"
      "36ec513ebb73c6c6b67e3c20129b83d07247307c3d58ba43d3"
      "e46d0e6af239f3ee2f0a35b99f8da0a3c6a57d0ea0ce7428ba"
      "b28fbd061d44c8f38e25291ff9c71f63dfacceae0321e63527"
      "3a22d0888387ebad489088d7efe0463592002545654694b875"
      "dd37d73c09037ed729d8b7dc9d228ac76701633fbb09fe7151"
      "de513b1b1b698c05a526dd4112aa81cd372790daaaef1e3abd"
      "242d00c0508e4803f669cd59d771fc502e17167369cf408e97"
      "f1df4ac3ea9287170745297e6aae5f2437b07097250d3c2e23"
      "800b0fc3ce36ee288502ff035b5247c10a83e82b6fba5da027"
      "a5a67b8958e44546da9271b7c6cc2fadfb2bd979ac9d4859b4"
      "d3a4c9209e6166d0b76f501fa3d92fbd750501e29c1df9aa8c"
      "3848282c80fb86a53cf53638093e0ffb29c8b38e84d533300b"
      "6845d5f8ef2936e86b654ee2006681ee23333861790bd8c75b"
      "7a4ccdd4f4ec9f671e7fece5c732ec02e80d4a7526b29904b6"
      "cf48ad38106d69f18b001fe17f6c976239019116e1d292fd21"
      "b954298e30567b78b322b68d467c6c9372a23ecc62b9b2b75f"
      "cdbf47bcbc5e5d9ce305bdc9e06c1cdf01969289003e61e242"
      "3a0e51322ee25110520552f52c17eea231b9c7f01c3eff85c1"
      "619ff825c3b7d6b8eab7b4adb82e33a88f7e9e2699d28ec6ce"
      "48de61dd043c02105831cf9e40f1b0f04f5262492d59a37c8f"
      "30fa4edd4c668e7db75ac9d9a244b08b5b5ade036b642f940a"
      "47519170d42e8b8ca1a79c8b35105f2b4361578c724807b938"
      "4a185cde9e79e5257fd9c5d8e808f4559413b6cc8a130bc0d6"
      "0f7ef58fb4c0c8201ed9e286b89df658c69c732f8cea9f0efa"
      "77e3c458d0dbdded1f946602e4b7c73839ac242bf8fca0a195"
      "66e8d077db36b7a899810372f66700b3a655065c2f972b437f"
      "7c0c125133fb1d7aafe13a630fbeda24e7ff5691e1534bfdbd"
      "d20903ade7422d0c078d3aea60460ec7a0ba89c5bc9cff2575"
      "af6de87f73b3cc77871f3d8c68b8e7d3b94c071cb0e56884ec"
      "bdde7f207ab4a2963dc6e386e26d8f2aa94801911afecdffbc"
      "2ca4591c47c9bdbceadc98275ea91ea0a9a3f1985a77c15ebd"
      "fd48050c321dcf4b1fb5296352279dcd83031e8451e83e9c53"
      "62711270026af1efec3d123504b3fe2f3ee5f162098e9b43b3"
      "ef408b4f838cc0cda5241400daca6b2001008eb451c85b8bcb"
      "29bbd4aecdd3041cb8f8037d559dce46f79b902998a1a1e333"
      "f389ed0d52c2c8d31e84d0b41f26d8785e473ddf758b4cf3ac"
      "b7d83878107699582771276c122271a71b5220d0cf68f5a941"
      "c768a1d404a84f5a103e0a8da215ccf05ccccc96fba492de5d"
      "6b11a62191275decf97723285132f3c19b7809cfc659b338d6"
      "5dc3f2f1950d45b213231c89d811e554174cf2d7aad6be7038"
      "72c7f1c02faf6267bae046a1293a53d827a1bf0278272be89d"
      "fa1fa85c7f6332865400b6873479d77c0d3c68fd242506c20e"
      "fdfce8a70a1d0f16d4f1a12b1dd0ab33778deaa3d7a168a371"
      "6ee09a6dc4b1d19c00423732d43a22ad978a9bbd2cf6c5c985"
      "a6666f7c3e2881013836d9280ecc3494bd0232629e608920ac"
      "c306ddb61da9b82ae8a3f0a1dd26de008f7f27704451a8d0dc"
      "e4016689ab6e066f6cbcb58ec4eb1b7d46761f510d3292f16d"
      "e8f07f513d7070c6785b3752a635ce48b81ffae016a145236f"
      "8c1d5c2fb94f10e4094bf9ef576e8974dd06d4ec5905f15086"
      "2a40c79ac15f0718c86cf61b3855d39614d940e4954bb61835"
      "b63ebd1f378f7ff4fe243800ecd1a98c2dc74438e618207f62"
      "ee0bf82c27b563de1b396c794868406645b13dfffd2eef8e4e"
      "dcd616427bdb88cf72225ca635b46e75cf7cca446440c8e490"
      "e7e742b7096c9c7177005d3bb8ece97806e21c55d5cf1d23a0"
      "32c3e86e6e22d2170c34d99b012d8551b6cbf565ac110d99ff"
      "eca2d84404fb855e0df4094fc04998ca30eff10ed73a67d810"
      "ef540b659bb08c3307514389c6b3c89d32746c08d24a44f815"
      "28cd6701716cd5c52ff207aacfaca17cbcaa42d079c70cb244"
      "6514a94cae89f18d5ed19963d6d4a65fb3490011362c5ee371"
      "14062c0cdf957f67818d8afd34f258d858e0531f2cc0514ead"
      "0d9a58e282bedf2722fa0f3ab3253d751e0f3adb5cedf67b1e"
      "865684708989162bad3ea168513df54976e41bd382f989f378"
      "65fbfb86dbdfdb04de5466c2b9ad5a54036f8fc0bb7336bfdf"
      "0bac117e40b2a083e68b1d4c6dc062c353e602db6d926437ee"
      "88813b554a3ca6f22436bd8f7577fbe99b795ea3ea81fd0852"
      "859052677983a2faedeb66bfbfa46b58fdaa294d6d67beff2c"
      "9d11d29c9a9e9d4e97b1c1307735b46bf04dcd2c7590e0df73"
      "77e0b738cc3e83519d1451b361ce805d85cd15a8e075f8b940"
      "424bc61dc9d2da15718c1be1e9ae618fc0d954592ebecddce0"
      "21114cfadf8ba8093d282fc9126d993f310d75d3c2430a9c06"
      "820a43cd69130e7d0e3d29d5bd3d9f974e765d919db72e99ae"
      "a45c98fe0abd15da1a50b549e5ad2d19f4db2a74079ded2173"
      "d66d4c696d9f5d1f72e2f98a0a5f7f9bec9a53e68e5083c154"
      "a5afa1705fbdae6cce3d995ad541cd9135ada17ebc81249091"
      "fa24aa760e783bf38e52aee5802f25973dabad3b4162d5acfd"
      "c7d870c04a551f6cff0d6a9541005ebcd07b390312fbd948de"
      "a8bc65deba9804758330dc7b149d01fe9032ce1b97b4d6d501"
      "d31fc6e2cb0abfac8b4d457081ffa614d3c149914bb28199b6"
      "8858d7bce5ffcdacbdc4a142fef87501946c63b1c750fb9d69"
      "93012e7eab85bc3083056c26dcbf8d6c92ddb23d9718378816"
      "004e218544f35eb5bb7dee0e396060e49c61897cce64ce419b"
      "e0d6639ace204bbedbcf0835bc713fb17b4cd521ad378417b2"
      "9074d28c1e75f0b5378b303256f08dad97b2c5584ad5f5217b"
      "eeeedb69e1813ee62891417c166f0ad7326be32925d573087a"
      "9f9f593feba6bf08dfbc4d4abd0783c05dee281255e7da959e"
      "a5d3d3c62b8a62ef397490d8da2c35c708ec7136905e52049c"
      "c408e1f82c299865406b2da8a72e7759099711518bc80f4d22"
      "27c6447abc91a4fdc6e97a578e4d70d2e48508f18eb03dba32"
      "3f4b2c02bc24bde1572c9ee8997c91accd1331b3b22147470d"
      "6587a7c343704f7fbb3048b70b7f6e96b4956b69ce4a425be4"
      "85548a1a99e2fed32d639776a5dc680802c871bef0c88e508c"
      "92390b96c5d7ebeb3d4d40a0c2a4b606ab8ea19d0b6009e59d"
      "3e90d731b100747c38250c7df11fe3cf693a46b88c27b93340"
      "0014a842ce5924ba9354343dc64306865fbede5c7650a192f4"
      "705eaf59bb80627abdfc70b0d1e6941312fe96eb9eb5bb9704"
      "a268dd4330e35348341001454a96e2a534261d1f4e88d0ba54"
      "0ff9818c2a26dc847335b0e831963de8ee5342aecccdbea1f9"
      "85b7445c123948835eb75705231fc485c02abbbe592b0c33f5"
      "257ee940742e92e43d3d901d6166de063ad9199ddb4da66955"
      "f1f7b4261a653452929c138465b6deb67c189ab8f27271ad3f"
      "ac1e82ee43394d38a920a1cb102014df9841e16c5a939478fc"
      "cf09dd6639a068c0dcae523c5a25eb316a03b1d6d5872b7938"
      "7694778f47e7341ccda18384a60866b2302023fc4e18179570"
      "ea41ff437d0b196153c5d82c5dee1b00a21284e90070f80aaa"
      "f6db25bea0711902528e1455c5ef810893857c25231dd0bb95"
      "b6d6a32d7501bd185f590f7211d5e70e5051f866ac33c84625"
      "3cf2449de94db274dde042aa436395bce16340bf94cd7da67b"
      "0bb63f61603c55a4811262936d2d6f338ee4af952beb557fc4"
      "ba052484fc95ec5e1d2c8d0fcffb93b65c138835363740234a"
      "641a1464231b23a105d65a16b40e73178b0476fd5998e52f27"
      "b6bdd08e971e4495593366a033b69e7094762de5e681980acb"
      "7cb8ec242b37701460ab017d99048b016b70f35579954a74a4"
      "cb01a102b5d2e160f8308781408ee92c7dbe2b3ae769f3a178"
      "b89163c96e533a99c5110e8ec37835393a218b2ca23ac7644e"
      "d37b747545c61882397ce603e6026ad0bef2fa946cda0d2a59"
      "a3e8afd8ece6dbd974b29c1cd98ec86ce98cf1f6db790bdcda"
      "3d5430f32dd00669bed51a5bac10b8ce64c38a385a82f17d6e"
      "496e48cb251942ffb5ae5dd998398157c098c9b04d9af00027"
      "ca17b70d46f3390bbf0ab84702c98167ef3750363a336eccc4"
      "00632ced942feab19c7c679d7a28638765ca961a791edf9670"
      "d02503a11958f566a231ce8f5cc5b8f7d335f3257e810e4f41"
      "3e1f305bbafa701a392da105d4c315ecdfe9ec45a8ed2fec77"
      "ed24be9b2149552ea90803f09cc7fa0a7c798baaadf9b1d7ca"
      "b44f4d1dc5a3cec9a20ce2ed4f67b0dc0d6fd8aaf1608116d0"
      "37feba862a1986dd3aa9a1307fd6993a43af8db68e170c1326"
      "6c3a40be8c9fe40df9d813ee3eb0e73e027ba7b3e2ad555d5d"
      "b542f2841b4aba58fd8cae4768225e039d46393b069c8c1a4f"
      "b6b8d109d17059eb790b2e1f88c2419836f372e80faff2d7b6"
      "69381470b5622fa11f98343d879b1046a972cd2e7334179def"
      "c1922942f1cd3fab0091693af023024bf49f82f74f3b06ff47"
      "36062a83c63c51f8c8ffd4ed49cc0611d04c013d3f54eab2a3"
      "5ff997c0214973b1de90e85389d59bc4c8cb1f8c16b5731952"
      "f2f5054e92487a138ef6bf60982b3b272984f0da2af7a547b9"
      "2aecb993540c889f02e5e7a5530d8361c507e498ab152b3b73"
      "56ae97cddcb67c17817df63c352ceb88c00460c6a4b9bf1bd2"
      "e5c95d10350e2f16bc7141815ae90c75895a951bcf64696f05"
      "3fc2bca511a9a624c8b67272724ee71a1247a51eb49d3b08ea"
      "9b108b751f99141c5cee292a22a9e5e40d9640bbebdfcece68"
      "d563df2a57c8404ae12f579bfa069b648be292581f0ea37a30"
      "34df97734bc9dc9814b18a74149d860531365273a1d60e7fe5"
      "d7d4a87e00de305e1d0089c3c9f347cfbb711532cd6e397ad3"
      "5dc60e2bd83145edc01bcc2f06443f51b5039a6315eb9c7f20"
      "9ae926c842caa8ad18d6a3bd62ac4d6f65153e52460405e492"
      "a1a866c0f5bdfeadda2c00efb3043158d2c6e1be58e939a77e"
      "94d1a9b16676141cf4657d8039cb668b939dfe63c67f6c21b1"
      "0424ccf80edee01ce02ccaa95c60edac675e08c86a09ee887c"
      "fb725f855ebbfe65c0204a4c15e9de808d047ada80923a17a9"
      "fbb3c69f9ebb7e6dc6efcac9fe2983d6b1c4d5bb82319cb1e6"
      "5c1a0358f94f12111a0ef891cc7af7b5965bb772951286aac3"
      "8d11b3b454d0f153fcfaa03f8526108eff7cf0cc77b308d44c"
      "552777713766f8ceec116e37714eb054f3191b6298be3ac743"
      "edcd071b23a680747a013c93ff92e12396318a6b56bde3c713"
      "6073dcee1dd8e7af29e9d6540c714c46305e857e41bd078b8e"
      "5ff3d609ee5a8d36ba9ac2cf58ce62da84d779ae0f699eff6a"
      "5a770e92088b2f65ee52347d91f493b79ffe3c6f76b999f32b"
      "122b12dca81a84bcb80cc009806c77c4ceb6b20cefeee1f64b"
      "d8ead5fc1591b325683785ac44b6c49aa10b483c69bf998a21"
      "efa40af7aae227a88cb040e4806ad29ea7683318687462fa19"
      "52c697ce9373bba2c3f4c6dc683fb52a7c61c9b2ec87246335"
      "d677f7d155843279e46f56cd13bed457b40771f964aa245cfd"
      "e7f0dcb9c58ed0aec35119f363ee526ca806ed404f3425e0fa"
      "c7da6606958c9f156516893da9df4fb4c7673f96daf9e6c990"
      "aaf09ca48c42c91b7d7753f9f2f278d65f578e9ecec1f52b65"
      "e9c5c6977e2a04434742ff8ce6873e225555da21a7902b60b2"
      "5d4352978050866e6f572eb00519944e68b16d60abf4a2d049"
      "980a2d7d97c103d1ab589d619b90fbd7210a99b9a82b0c32e4"
      "e01b8a0099e762c5676e68419187468fceb8aedf7e1fc0cfc7"
      "1a1212ee54d1177e8d61e9ea391421371b57fc522c256e7f57"
      "04909f7a677ad8c349f19198a0d1e343f8fddf40e0853e71f2"
      "07d5a9ca9f6a882ea8a67d39b7c4237fb1f7f3f9c644d5e468"
      "df85ed5de9373abcc8a7fcb4b91208b5d80c5e3305351e120e"
      "0166f18cc179bab703e1ef0d9a3f1edd3e8a4fa1f8336abcc1"
      "899a4d2f67d9a3b3d8507d2d5dd9845651d528a5292936b62c"
      "49c60396454bd2019419a1f3693c781a7628038f05735ae859"
      "98ec14f2b72d89c94ed01fbf5fca09afa7788be289c32cae4e"
      "217511d3067b7d9e0c27e0110";
};

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
