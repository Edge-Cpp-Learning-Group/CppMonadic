#include <catch2/catch_test_macros.hpp>
#include "../task.h"

TEST_CASE("Execute task", "[Task]") {
  int val = 0;

  auto task = ITask<int>::Make(1);
  task->Execute([&val](int n) { val = n; });

  REQUIRE(val == 1);
}
