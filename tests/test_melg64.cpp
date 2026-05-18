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

const melg64::result_type init_key_raw[4] = {0x12345UL, 0x23456UL, 0x34567UL,
                                             0x45678UL};

const std::array<melg64::result_type, 4> init_key_array =
    std::to_array(init_key_raw);

const std::vector<melg64::result_type> init_key_vector(init_key_array.begin(),
                                                       init_key_array.end());

template <std::uniform_random_bit_generator URBG>
bool test_known_output(URBG& engine,
                       std::span<const melg64::result_type> expected) {
  for (auto e : expected) {
    if (engine() != e) return false;
  }

  return true;
}

bool test_known_output_melg607(std::span<const melg64::result_type> init_key) {
  std::vector<melg64::result_type> expected;

  std::ifstream ifs("tests/melg607-64.out");

  if (!ifs.is_open()) {
    throw std::runtime_error("failed to open melg607-64.out");
  }

  melg64::result_type receiver;

  std::string header;

  std::getline(ifs, header);  // skip header

  while (ifs >> receiver) {
    expected.push_back(receiver);
    if (expected.size() == 1000) break;
  }

  melg64::melg607 engine(init_key);

  return test_known_output(engine, expected);
}

bool test_known_output_melg1279(std::span<const melg64::result_type> init_key) {
  static constexpr melg64::result_type expected[20] = {
      16235135108973359505UL, 12114426808952376689UL, 17843685570748579801UL,
      1801320348860028384UL,  650442017251097059UL,   7401930806073658224UL,
      8544538885320907937UL,  10680173795930056254UL, 7594459215165978320UL,
      16930061427514290611UL, 6161988295406803453UL,  15301168040311454419UL,
      1510765571013867513UL,  51246976282527744UL,    1815788032190076904UL,
      17209382128667908794UL, 1425032498633941855UL,  18317445030881500124UL,
      14443076587925727999UL, 2993771411211919914UL};

  melg64::melg1279 engine(init_key);

  return test_known_output(engine, expected);
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
       []() { return test_known_output_melg1279(init_key_vector); }}};

  int count_failed = 0;

  for (const auto& test : tests) {
    const bool result = test.func();
    std::cout << (result ? "PASS" : "FAIL") << ": " << test.name << std::endl;
    if (!result) count_failed++;
  }

  return count_failed;
}
