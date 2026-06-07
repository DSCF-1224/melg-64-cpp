# melg-64-cpp

C++ header-only adaptation of the MELG family — 64-bit maximally equidistributed
F<sub>2</sub>-linear generators with Mersenne prime period.

[![CI](https://github.com/DSCF-1224/melg-64-cpp/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/DSCF-1224/melg-64-cpp/actions/workflows/cmake-multi-platform.yml)

## Requirements

- C++20

## Installation

Copy `include/melg64/melg64.hpp` into your project, or add this repository
as a subdirectory and link via CMake:

```cmake
add_subdirectory(melg-64-cpp)
target_link_libraries(your_target PRIVATE melg64)
```

## Generators

| Type alias          | Period                |
|:--------------------|:----------------------|
| `melg64::melg607`   | 2<sup>607</sup> - 1   |
| `melg64::melg1279`  | 2<sup>1279</sup> - 1  |
| `melg64::melg2281`  | 2<sup>2281</sup> - 1  |
| `melg64::melg4253`  | 2<sup>4253</sup> - 1  |
| `melg64::melg11213` | 2<sup>11213</sup> - 1 |
| `melg64::melg19937` | 2<sup>19937</sup> - 1 |
| `melg64::melg44497` | 2<sup>44497</sup> - 1 |

All types satisfy `std::uniform_random_bit_generator` and are interchangeable with standard library distributions.

## Usage

See the [`examples/`](examples/) directory for usage examples.

## License and Attribution

This library is a C++ adaptation of the original C implementation by
Shin Harase (Ritsumeikan University) and Takamitsu Kimoto.

**Usage terms** (inherited from the original):
free for personal, academic, and non-commercial use.
For commercial use, contact harase @ fc.ritsumei.ac.jp.

**Bug reports**: please file issues in this repository's issue tracker.

## Reference

> S. Harase and T. Kimoto, "Implementing 64-bit maximally equidistributed **F**<sub>2</sub>-linear generators with Mersenne prime period", ACM Transactions on Mathematical Software, Volume 44, Issue 3, April 2018, Article No. 30, 11 pp. <a href="http://doi.acm.org/10.1145/3159444">Article</a>

[sharase/melg-64: Implementing 64-bit Maximally Equidistributed F2-Linear Generators with Mersenne Prime Period](https://github.com/sharase/melg-64)
