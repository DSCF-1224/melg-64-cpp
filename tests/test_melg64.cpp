#include <array>
// https://cppreference.com/cpp/header/array
// https://cpprefjp.github.io/reference/array.html

#include <cstdlib>
// https://cppreference.com/cpp/header/cstdlib
// https://cppreference.com/cpp/utility/program/EXIT_status
// https://cpprefjp.github.io/reference/cstdlib.html
// https://cpprefjp.github.io/reference/cstdlib/exit_success.html

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

#include <string_view>
// https://cppreference.com/cpp/header/string_view
// https://cpprefjp.github.io/reference/string_view.html

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

const std::array<melg64::result_type, 4> init_key_a = {0x12345UL, 0x23456UL,
                                                       0x34567UL, 0x45678UL};

bool test_known_output_melg607(std::span<const melg64::result_type> init_key) {
  static constexpr melg64::result_type expected[10] = {
      12495950309458289112UL, 8163910988915845065UL,  17447112683145787935UL,
      14392119910362097645UL, 7164909824801924305UL,  17038754296801418064UL,
      10871240116890307231UL, 12692713980656253045UL, 10435959733805108698UL,
      5542897018756383954UL};

  melg64::melg607 engine(init_key);

  for (auto e : expected) {
    if (engine() != e) return false;
  }

  return true;
}

struct Test {
  const char* name;
  bool (*func)();
};

int main(void) {
  PrintBuildInfo();

  const Test tests[] = {{"known_output_melg607(std::array)", []() {
                           return test_known_output_melg607(init_key_a);
                         }}};

  return EXIT_SUCCESS;
}
