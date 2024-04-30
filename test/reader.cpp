#include <catch2/catch_test_macros.hpp>
#include <string>
#include "../reader.h"
#include "../pure-bind-monad.h"

TEST_CASE("As pure function", "[Reader]") {
  auto r1 = Reader<int>::To<int>([](int n) { return n + 1; });
  REQUIRE(r1(1) == 2);
}

TEST_CASE("Pure", "[Reader]") {
  auto r1 = Reader<int>::To<int>::Pure(42);
  REQUIRE(r1(1) == 42);
  REQUIRE(r1(10) == 42);

  auto r2 = Monad<Reader<int>::To>::Pure(42);
  REQUIRE(r2(1) == 42);
  REQUIRE(r2(10) == 42);
}

TEST_CASE("Bind", "[Reader]") {
  auto r1 = Reader<int>::To<int>([](int n) { return n + 1; });
  auto r2 = Reader<int>::To<int>::Bind(r1, [](int n) {
    return Reader<int>::To<int>([=](int m) {
      return m + n;
    });
  });
  REQUIRE(r2(3) == 7);

  auto r3 = Monad<Reader<int>::To>::Bind(r1, [](int n) {
    return Reader<int>::To<int>([=](int m) {
      return m * n;
    });
  });
  REQUIRE(r3(3) == 12);
}

TEST_CASE("Map", "[Reader]") {
  auto r1 = Reader<int>::To<int>([](int n) { return n + 1; });
  auto r2 = Monad<Reader<int>::To>::Map([](int n) { return std::to_string(n); }, r1);

  REQUIRE(r2(5) == "6");
}

TEST_CASE("Join", "[Reader]") {
  auto r1 = Reader<int>::To<Reader<int>::To<std::string>>([](int n) {
    return [=](int m) { return std::to_string(n * m + 1); };
  });
  auto r2 = Monad<Reader<int>::To>::Join(r1);

  REQUIRE(r2(5) == "26");
}

TEST_CASE("Lift", "[Reader]") {
  auto rA = Reader<int>::To<int>([](int n) { return n + 1; });
  auto rB = Reader<int>::To<int>([](int n) { return n * 2; });
  auto rC = Reader<int>::To<int>([](int n) { return n * n; });

  auto rD = Monad<Reader<int>::To>::Lift([](int a, int b, int c) { return a + b + c; }, rA, rB, rC);

  REQUIRE(rD(1) == 5);
  REQUIRE(rD(2) == 11);
}
