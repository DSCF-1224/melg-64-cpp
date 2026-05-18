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
// https://cppreference.com/cpp/numeric/random/uniform_random_bit_generator
// https://cpprefjp.github.io/reference/random.html
// https://cpprefjp.github.io/reference/random/uniform_random_bit_generator.html

#include <span>
// https://cppreference.com/cpp/header/span
// https://cpprefjp.github.io/reference/span.html

#include <stdexcept>
// https://cppreference.com/cpp/header/stdexcept
// https://cpprefjp.github.io/reference/stdexcept.html

#include <string>
// https://cppreference.com/cpp/header/string
// https://cpprefjp.github.io/reference/string.html

#include <string_view>
// https://cppreference.com/cpp/header/string_view
// https://cpprefjp.github.io/reference/string_view.html

#include <vector>
// https://cppreference.com/cpp/header/vector
// https://cpprefjp.github.io/reference/vector.html

#include <melg64/melg64.hpp>
// target of this test

#include "build_info.hpp"

void PrintBuildInfo() {
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

/* test: known output */

const melg64::result_type init_key_raw[4] = {0x12345UL, 0x23456UL, 0x34567UL,
                                             0x45678UL};

const std::array<melg64::result_type, 4> init_key_array =
    std::to_array(init_key_raw);

const std::vector<melg64::result_type> init_key_vector(init_key_array.begin(),
                                                       init_key_array.end());

template <std::uniform_random_bit_generator URBG>
bool test_known_output(URBG& engine, const char* file) {
  std::ifstream ifs(std::string("tests/") + file);

  if (!ifs.is_open()) {
    throw std::runtime_error(std::string("failed to open ") + file);
  }

  constexpr std::size_t sample_size = 1000;

  static_assert(sample_size > 0);

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

  bool succeeded;

  for (std::size_t i = 0; i < sample_size; i++) {
    const melg64::result_type harvest = engine();

    succeeded = (harvest == expected[i]);

    if (!succeeded) {
      std::cout << "  index    : " << i << std::endl
                << "  expected : " << expected[i] << std::endl
                << "  harvest  : " << harvest << std::endl;

      return succeeded;
    }
  }

  return succeeded;
}

bool test_known_output_melg607(std::span<const melg64::result_type> init_key) {
  melg64::melg607 engine(init_key);

  return test_known_output(engine, "melg607-64.out");
}

bool test_known_output_melg1279(std::span<const melg64::result_type> init_key) {
  melg64::melg1279 engine(init_key);

  return test_known_output(engine, "melg1279-64.out");
}

bool test_known_output_melg2281(std::span<const melg64::result_type> init_key) {
  melg64::melg2281 engine(init_key);

  return test_known_output(engine, "melg2281-64.out");
}

bool test_known_output_melg4253(std::span<const melg64::result_type> init_key) {
  melg64::melg4253 engine(init_key);

  return test_known_output(engine, "melg4253-64.out");
}

bool test_known_output_melg11213(
    std::span<const melg64::result_type> init_key) {
  melg64::melg11213 engine(init_key);

  return test_known_output(engine, "melg11213-64.out");
}

bool test_known_output_melg19937(
    std::span<const melg64::result_type> init_key) {
  melg64::melg19937 engine(init_key);

  return test_known_output(engine, "melg19937-64.out");
}

bool test_known_output_melg44497(
    std::span<const melg64::result_type> init_key) {
  melg64::melg44497 engine(init_key);

  return test_known_output(engine, "melg44497-64.out");
}

/* test: default constructor */

template <std::uniform_random_bit_generator URBG>
bool test_default_constructor() {
  URBG a;
  URBG b(URBG::default_seed);
  return (a == b);
}

struct Test {
  const char* name;
  bool (*func)();
};

int main(void) {
  PrintBuildInfo();

  const Test tests[] = {
      {"known_output_melg607(raw array)",
       []() { return test_known_output_melg607(init_key_raw); }},
      {"known_output_melg607(std::array)",
       []() { return test_known_output_melg607(init_key_array); }},
      {"known_output_melg607(std::vector)",
       []() { return test_known_output_melg607(init_key_vector); }},
      {"known_output_melg1279(raw array)",
       []() { return test_known_output_melg1279(init_key_raw); }},
      {"known_output_melg1279(std::array)",
       []() { return test_known_output_melg1279(init_key_array); }},
      {"known_output_melg1279(std::vector)",
       []() { return test_known_output_melg1279(init_key_vector); }},
      {"known_output_melg2281(raw array)",
       []() { return test_known_output_melg2281(init_key_raw); }},
      {"known_output_melg2281(std::array)",
       []() { return test_known_output_melg2281(init_key_array); }},
      {"known_output_melg2281(std::vector)",
       []() { return test_known_output_melg2281(init_key_vector); }},
      {"known_output_melg4253(raw array)",
       []() { return test_known_output_melg4253(init_key_raw); }},
      {"known_output_melg4253(std::array)",
       []() { return test_known_output_melg4253(init_key_array); }},
      {"known_output_melg4253(std::vector)",
       []() { return test_known_output_melg4253(init_key_vector); }},
      {"known_output_melg11213(raw array)",
       []() { return test_known_output_melg11213(init_key_raw); }},
      {"known_output_melg11213(std::array)",
       []() { return test_known_output_melg11213(init_key_array); }},
      {"known_output_melg11213(std::vector)",
       []() { return test_known_output_melg11213(init_key_vector); }},
      {"known_output_melg19937(raw array)",
       []() { return test_known_output_melg19937(init_key_raw); }},
      {"known_output_melg19937(std::array)",
       []() { return test_known_output_melg19937(init_key_array); }},
      {"known_output_melg19937(std::vector)",
       []() { return test_known_output_melg19937(init_key_vector); }},
      {"known_output_melg44497(raw array)",
       []() { return test_known_output_melg44497(init_key_raw); }},
      {"known_output_melg44497(std::array)",
       []() { return test_known_output_melg44497(init_key_array); }},
      {"known_output_melg44497(std::vector)",
       []() { return test_known_output_melg44497(init_key_vector); }},
      {"default_constructor_melg607",
       []() { return test_default_constructor<melg64::melg607>(); }},
      {"default_constructor_melg1279",
       []() { return test_default_constructor<melg64::melg1279>(); }},
      {"default_constructor_melg2281",
       []() { return test_default_constructor<melg64::melg2281>(); }},
      {"default_constructor_melg4253",
       []() { return test_default_constructor<melg64::melg4253>(); }},
      {"default_constructor_melg11213",
       []() { return test_default_constructor<melg64::melg11213>(); }},
      {"default_constructor_melg19937",
       []() { return test_default_constructor<melg64::melg19937>(); }}};

  int count_failed = 0;

  for (const auto& test : tests) {
    const bool result = test.func();
    std::cout << (result ? "PASS" : "FAIL") << ": " << test.name << std::endl;
    if (!result) count_failed++;
  }

  return count_failed;
}
