// Fixture probe: a C++20 feature the dialect must accept.
#include <concepts>
template <std::integral T> constexpr T twice(T v) { return v * 2; }
int main() { return twice(0); }
