#include <catch2/catch_test_macros.hpp>
#include "../single-execution.h"

TEST_CASE("SingleExecution", "[SingleExecution]") {
  int n = 10;

  auto se1 = SingleExecution([&n]() { return ++n; });
  se1();
  REQUIRE(n == 11);
  se1();
  REQUIRE(n == 11);
  {
    auto se2 = SingleExecution([&n]() { return ++n; });
    REQUIRE(n == 11);
  }
  REQUIRE(n == 12);

  auto se3 = SingleExecution([&n]() { return ++n; });
  {
    SingleExecution se4(std::move(se3));
    REQUIRE(n == 12);
  }
  REQUIRE(n == 13);
  se3();
  REQUIRE(n == 13);
}
