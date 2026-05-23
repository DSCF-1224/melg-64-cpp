#include "test_melg64_common.hpp"

int main(void) {
  PrintBuildInfo();

  const Test tests[] = {
      {"known_output_melg44497(raw array)",
       test_known_output_raw<melg64::melg44497>},
      {"known_output_melg44497(std::array)",
       test_known_output_array<melg64::melg44497>},
      {"known_output_melg44497(std::vector)",
       test_known_output_vector<melg64::melg44497>},
      {"default_constructor_melg44497",
       test_default_constructor<melg64::melg44497>},
      {"seed_reset_melg44497", test_seed_reset<melg64::melg44497>},
      {"seed_reset_melg44497(raw array)",
       test_seed_reset_raw<melg64::melg44497>},
      {"seed_reset_melg44497(std::array)",
       test_seed_reset_array<melg64::melg44497>},
      {"seed_reset_melg44497(std::vector)",
       test_seed_reset_vector<melg64::melg44497>}};

  int count_failed = 0;

  for (const auto& test : tests) {
    const bool result = test.func();
    std::cout << (result ? "PASS" : "FAIL") << ": " << test.name << std::endl;
    if (!result) count_failed++;
  }

  return count_failed;
}
