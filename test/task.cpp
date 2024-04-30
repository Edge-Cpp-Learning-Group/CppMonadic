#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include "../task2.h"

TEST_CASE("Execute task", "[Task]") {
  int from = 1;
  int to = 0;

  auto task = Task<int>([from](Task<int>::Callback cb) {
    return cb(from);
  });

  task([&to](int n) {
    return [&to, n](){ to = n; };
  });

  REQUIRE(to == 1);
}
