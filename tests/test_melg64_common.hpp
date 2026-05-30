#ifndef TEST_MELG64_COMMON_H_
#define TEST_MELG64_COMMON_H_

#include <array>
// https://cppreference.com/cpp/header/array
// https://cppreference.com/cpp/container/array/to_array
// https://cpprefjp.github.io/reference/array.html
// https://cpprefjp.github.io/reference/array/to_array.html

#include <cstdlib>
// https://cppreference.com/cpp/header/cstdlib
// https://cppreference.com/cpp/utility/program/EXIT_status
// https://cpprefjp.github.io/reference/cstdlib.html
// https://cpprefjp.github.io/reference/cstdlib/exit_success.html

#include <fstream>
// https://cppreference.com/cpp/header/fstream
// https://cpprefjp.github.io/reference/fstream.html

#include <iostream>
// https://cppreference.com/cpp/header/iostream
// https://cppreference.com/cpp/io/cout
// https://cppreference.com/cpp/io/manip/endl
// https://cpprefjp.github.io/reference/iostream.html
// https://cpprefjp.github.io/reference/iostream/cout.html
// https://cpprefjp.github.io/reference/ostream/endl.html

#include <random>
// https://cppreference.com/cpp/header/random
// https://cppreference.com/cpp/numeric/random/random_device
// https://cppreference.com/cpp/numeric/random/uniform_random_bit_generator
// https://cpprefjp.github.io/reference/random.html
// https://cpprefjp.github.io/reference/random/random_device.html
// https://cpprefjp.github.io/reference/random/uniform_random_bit_generator.html

#include <span>
// https://cppreference.com/cpp/header/span
// https://cpprefjp.github.io/reference/span.html

#include <stdexcept>
// https://cppreference.com/cpp/header/stdexcept
// https://cppreference.com/cpp/error/runtime_error
// https://cpprefjp.github.io/reference/stdexcept.html

#include <string>
// https://cppreference.com/cpp/header/string
// https://cppreference.com/cpp/string/basic_string
// https://cpprefjp.github.io/reference/string.html
// https://cpprefjp.github.io/reference/string/basic_string.html

#include <string_view>
// https://cppreference.com/cpp/header/string_view
// https://cpprefjp.github.io/reference/string_view.html

#include <vector>
// https://cppreference.com/cpp/header/vector
// https://cppreference.com/cpp/container/vector
// https://cpprefjp.github.io/reference/vector.html
// https://cpprefjp.github.io/reference/vector/vector.html

#include <melg64/melg64.hpp>
// target of this test

#include "build_info.hpp"

void print_build_info() {
  std::cout << "Compiler ID      : " << BUILD_COMPILER_ID << std::endl;
  std::cout << "Compiler Version : " << BUILD_COMPILER_VERSION << std::endl;
  std::cout << "std              : C++" << BUILD_CXX_STANDARD << std::endl;
  std::cout << "Build Type       : " << BUILD_BUILD_TYPE << std::endl;
  std::cout << "CXX Flags        : " << BUILD_CXX_FLAGS << std::endl;

  const std::string_view build_type = BUILD_BUILD_TYPE;

  if (!build_type.empty()) {
    std::cout << "Type Flags       : ";
  } else {
    return;
  }

  if (build_type == "Debug") {
    std::cout << BUILD_CXX_FLAGS_DEBUG;
  } else if (build_type == "Release") {
    std::cout << BUILD_CXX_FLAGS_RELEASE;
  } else if (build_type == "RelWithDebInfo") {
    std::cout << BUILD_CXX_FLAGS_RELWITHDEBINFO;
  } else if (build_type == "MinSizeRel") {
    std::cout << BUILD_CXX_FLAGS_MINSIZEREL;
  }

  std::cout << std::endl << std::endl;
}

/* test: default constructor */

template <std::uniform_random_bit_generator URBG>
bool test_default_constructor() {
  URBG a;
  URBG b(URBG::default_seed);
  return (&a != &b) && (a == b);
}

/* test: jump idempotence */

template <std::uniform_random_bit_generator URBG>
bool test_jump_idempotent() {
  std::random_device seed_source;

  const melg64::result_type s = static_cast<melg64::result_type>(seed_source());

  URBG a(s), b(s);

  if (&a == &b) {
    return false;
  }

  a.jump();
  b.jump();

  const bool failed = (a != b);

  if (failed) {
    std::cout << "s: " << s << std::endl;
  }

  return !failed;
}

/* test: known output */

const melg64::result_type init_key_raw[4] = {0x12345UL, 0x23456UL, 0x34567UL,
                                             0x45678UL};

const std::array<melg64::result_type, 4> init_key_array =
    std::to_array(init_key_raw);

const std::vector<melg64::result_type> init_key_vector(init_key_array.begin(),
                                                       init_key_array.end());

template <std::uniform_random_bit_generator URBG>
bool compare_output(URBG& engine,
                    const std::vector<melg64::result_type>& expected) {
  for (std::size_t i = 0; i < expected.size(); i++) {
    const melg64::result_type harvest = engine();

    if (harvest != expected[i]) {
      std::cout << "  index    : " << i << std::endl
                << "  expected : " << expected[i] << std::endl
                << "  harvest  : " << harvest << std::endl;

      return false;
    }
  }

  return true;
}

template <std::uniform_random_bit_generator URBG>
bool test_known_output_impl(std::span<const melg64::result_type> init_key,
                            const char* file) {
  std::ifstream ifs(std::string("tests/") + file);

  if (!ifs.is_open()) {
    throw std::runtime_error(std::string("failed to open ") + file);
  }

  constexpr std::size_t sample_size = 1000;

  static_assert(sample_size > 0);

  // Skip floating-point output section and section headers.
  // 1000 outputs of genrand64_res53 + blank lines + headers = 206 lines
  constexpr std::size_t lines_to_skip = 206;

  melg64::result_type receiver;

  std::vector<melg64::result_type> expected;

  std::string header;

  std::getline(ifs, header);  // skip header

  while (ifs >> receiver) {
    expected.push_back(receiver);
    if (expected.size() == sample_size) break;
  }

  if (expected.size() < sample_size) {
    throw std::runtime_error(std::string("insufficient data in ") + file);
  }

  URBG engine(init_key);

  if (!compare_output(engine, expected)) return false;

  expected.clear();

  for (std::size_t i = 0; i < lines_to_skip; i++) std::getline(ifs, header);

  while (ifs >> receiver) {
    expected.push_back(receiver);
    if (expected.size() == sample_size) break;
  }

  if (expected.size() < sample_size) {
    throw std::runtime_error(std::string("insufficient data in ") + file);
  }

  for (std::size_t i = 0; i < sample_size; i++) engine();

  engine.jump();

  if (!compare_output(engine, expected)) return false;

  return true;
}

template <std::uniform_random_bit_generator URBG>
bool test_known_output(std::span<const melg64::result_type> init_key) {
  const char* file = nullptr;

  if constexpr (std::is_same_v<URBG, melg64::melg607>) {
    file = "melg607-64.out";
  } else if constexpr (std::is_same_v<URBG, melg64::melg1279>) {
    file = "melg1279-64.out";
  } else if constexpr (std::is_same_v<URBG, melg64::melg2281>) {
    file = "melg2281-64.out";
  } else if constexpr (std::is_same_v<URBG, melg64::melg4253>) {
    file = "melg4253-64.out";
  } else if constexpr (std::is_same_v<URBG, melg64::melg11213>) {
    file = "melg11213-64.out";
  } else if constexpr (std::is_same_v<URBG, melg64::melg19937>) {
    file = "melg19937-64.out";
  } else if constexpr (std::is_same_v<URBG, melg64::melg44497>) {
    file = "melg44497-64.out";
  } else {
    static_assert(false, "unsupported melg64 type");
  }

  return test_known_output_impl<URBG>(init_key, file);
}

template <std::uniform_random_bit_generator URBG>
bool test_known_output_raw(void) {
  return test_known_output<URBG>(init_key_raw);
}

template <std::uniform_random_bit_generator URBG>
bool test_known_output_array(void) {
  return test_known_output<URBG>(init_key_array);
}

template <std::uniform_random_bit_generator URBG>
bool test_known_output_vector(void) {
  return test_known_output<URBG>(init_key_vector);
}

/* test: reset by `seed()` */

template <std::uniform_random_bit_generator URBG>
bool test_seed_reset() {
  std::random_device seed_source;

  const melg64::result_type s = static_cast<melg64::result_type>(seed_source());

  URBG a(s), b(s);

  if (&a == &b) return false;

  for (std::size_t i = 0; i < 100; i++) {
    a();
  }

  if (a == b) {
    std::cout << "s: " << s << std::endl;
    return false;
  }

  a.seed(s);

  return (a == b);
}

template <std::uniform_random_bit_generator URBG>
bool test_seed_reset(std::span<const melg64::result_type> init_key) {
  URBG a(init_key), b(init_key);

  if (&a == &b) return false;

  for (std::size_t i = 0; i < 100; i++) {
    a();
  }

  if (a == b) return false;

  a.seed(init_key);

  return (a == b);
}

template <std::uniform_random_bit_generator URBG>
bool test_seed_reset_raw() {
  return test_seed_reset<URBG>(init_key_raw);
}

template <std::uniform_random_bit_generator URBG>
bool test_seed_reset_array() {
  return test_seed_reset<URBG>(init_key_array);
}

template <std::uniform_random_bit_generator URBG>
bool test_seed_reset_vector() {
  return test_seed_reset<URBG>(init_key_vector);
}

struct Test {
  const char* name;
  bool (*func)();
};

/* test function for each variant */
template <std::uniform_random_bit_generator URBG>
int test_runner() {
  print_build_info();

  const Test tests[] = {
      {"known output (raw array)", test_known_output_raw<URBG>},
      {"known output (std::array)", test_known_output_array<URBG>},
      {"known output (std::vector)", test_known_output_vector<URBG>},
      {"default constructor", test_default_constructor<URBG>},
      {"jump idempotence", test_jump_idempotent<URBG>},
      {"seed reset", test_seed_reset<URBG>},
      {"seed reset (raw array)", test_seed_reset_raw<URBG>},
      {"seed reset (std::array)", test_seed_reset_array<URBG>},
      {"seed reset (std::vector)", test_seed_reset_vector<URBG>}};

  int count_failed = 0;

  for (const auto& test : tests) {
    const bool result = test.func();
    std::cout << (result ? "PASS" : "FAIL") << ": " << test.name << std::endl;
    if (!result) count_failed++;
  }

  return count_failed;
}

#endif /* TEST_MELG64_COMMON_H_ */
