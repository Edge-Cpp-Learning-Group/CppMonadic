#include <catch2/catch_test_macros.hpp>
#include <string>
#include "../effect.h"
#include "../pure-bind-monad.h"
#include "../single-execution.h"

TEST_CASE("Execute effect", "[Effect]") {
  int n = 10;
  auto eff1 = Effect<int>([&]() { return n++; });
  REQUIRE(eff1() == 10);
  REQUIRE(n == 11);
  REQUIRE(eff1() == 11);
  REQUIRE(n == 12);
}

TEST_CASE("Effect Pure", "[Effect]") {
  auto eff1 = Effect<int>::Pure(123);
  REQUIRE(eff1() == 123);
}

TEST_CASE("Effect Bind", "[Effect]") {
  int n = 10;
  auto eff1 = Effect<int>([&]() { return n++; });
  auto eff2 = Effect<int>::Bind(eff1, [&](int v) {
    return Effect<int>([v, &n]() { return (n++) + v; });
  });
  REQUIRE(eff2() == 21);
  REQUIRE(n == 12);

  auto eff3 = Monad<Effect>::Bind(eff2, [](int v) { return Monad<Effect>::Pure(v * 2); });
  REQUIRE(eff3() == 50);
  REQUIRE(n == 14);
}

TEST_CASE("Effect Map", "[Effect]") {
  int n = 10;
  auto eff1 = Effect<int>([&]() { return n++; });
  auto eff2 = Monad<Effect>::Map([&](int v) {
    return std::to_string(v);
  }, eff1);
  REQUIRE(eff2() == "10");
  REQUIRE(n == 11);
  REQUIRE(eff2() == "11");
  REQUIRE(n == 12);
}

TEST_CASE("Effect Lift", "[Effect]") {
  int a = 10;
  int b = 20;
  int c = 30;

  auto effIncA = Effect<int>([&a]() { return ++a; });
  auto effIncB = Effect<int>([&b]() { return ++b; });
  auto effIncC = Effect<int>([&c]() { return ++c; });

  auto effIncAll = Monad<Effect>::Lift([](int a, int b, int c) {
    return a + b + c;
  }, effIncA, effIncB, effIncC);

  REQUIRE(effIncAll() == 63);
  REQUIRE(a == 11);
  REQUIRE(b == 21);
  REQUIRE(c == 31);
}

TEST_CASE("SingleExecution with Effect", "[Effect]") {
  int n = 10;

  auto effInc1 = Effect(SingleExecution([&n]() { ++n; }));
  effInc1();
  REQUIRE(n == 11);
  effInc1();
  REQUIRE(n == 11);

  {
    auto effInc2 = Effect(SingleExecution([&n]() { ++n; }));
    auto effInc3 = effInc2;
    REQUIRE(n == 11);
  }
  REQUIRE(n == 12);
}
