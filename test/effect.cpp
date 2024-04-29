#include <catch2/catch_test_macros.hpp>
#include "../effect.h"

TEST_CASE("Execute effect", "[IEffect]") {
  int n = 10;
  auto eff1 = Effect<int>([&]() { return n++; });
  REQUIRE(eff1() == 10);
  REQUIRE(n == 11);
  REQUIRE(eff1() == 11);
  REQUIRE(n == 12);
}

TEST_CASE("Effect Pure", "[IEffect]") {
  auto eff1 = Effect<int>::Pure(123);
  REQUIRE(eff1() == 123);
}

TEST_CASE("Effect Bind", "[IEffect]") {
  int n = 10;
  auto eff1 = Effect<int>([&]() { return n++; });
  auto eff2 = Effect<int>::Bind(eff1, [&](int v) {
    return Effect<int>([v, &n]() { return (n++) + v; });
  });
  REQUIRE(eff2() == 21);
  REQUIRE(n == 12);
}
