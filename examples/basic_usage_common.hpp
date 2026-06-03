#ifndef EXAMPLE_MELG64_COMMON_H_
#define EXAMPLE_MELG64_COMMON_H_

#include <cstdlib>
// https://cppreference.com/cpp/header/cstdlib
// https://cppreference.com/cpp/utility/program/EXIT_status
// https://cpprefjp.github.io/reference/cstdlib.html
// https://cpprefjp.github.io/reference/cstdlib/exit_success.html
// https://cpprefjp.github.io/reference/cstdlib/exit_failure.html

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

#include <string_view>
// https://cppreference.com/cpp/header/string_view
// https://cpprefjp.github.io/reference/string_view.html

#include <melg64/melg64.hpp>
// target of this example

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

/**
 * @brief output the generated values
 */
template <std::uniform_random_bit_generator URBG>
void output_generated_values(URBG& engine) {
  for (std::size_t i = 0; i < 5; i++) {
    std::cout << i << " " << engine() << std::endl;
  }
}

/**
 * @brief default seed
 */
template <std::uniform_random_bit_generator URBG>
void example_default_seed() {
  URBG engine;

  output_generated_values(engine);
}

/**
 * @brief zero seed
 */
template <std::uniform_random_bit_generator URBG>
void example_zero_seed() {
  URBG engine(static_cast<melg64::result_type>(0));

  output_generated_values(engine);
}

// example of array seed (raw array)

/**
 * @brief array seed (raw array)
 */
template <std::uniform_random_bit_generator URBG>
void example_array_seed_raw() {
  const melg64::result_type init_key_raw[4] = {0x12345UL, 0x23456UL, 0x34567UL,
                                               0x45678UL};

  URBG engine(init_key_raw);

  output_generated_values(engine);
}

struct Example {
  const char* name;
  void (*func)();
};

/* test function for each variant */
template <std::uniform_random_bit_generator URBG>
int example_runner() {
  print_build_info();

  const Example examples[] = {
      {"default seed", example_default_seed<URBG>},
      {"zero seed", example_zero_seed<URBG>},
      {"array seed (raw array)", example_array_seed_raw<URBG>}};

  for (const auto& example : examples) {
    try {
      example.func();
    } catch (const std::exception& exception) {
      std::cerr << "FAIL : " << example.name << std::endl;
      std::cerr << exception.what() << std::endl;
      return EXIT_FAILURE;
    }

    std::cout << "PASS : " << example.name << std::endl;
  }

  return EXIT_SUCCESS;
}

#endif /* EXAMPLE_MELG64_COMMON_H_ */
