#include <catch2/catch_test_macros.hpp>
#include "../monad.h"
#include "../maybe.h"

TEST_CASE("Maybe as Monad", "[Maybe]") {
  auto x = Maybe<int>(0);
  auto y = Maybe<int>();
  auto z = Maybe<int>(2);

  auto w = Monad<Maybe>::Lift([](int x, int y, int z) {
    return x + y + z;
  }, x, y, z);

  REQUIRE(w.HasValue() == false);

  auto u = Monad<Maybe>::Lift([](int x, int z) {
    return x + z;
  }, x, z);

  REQUIRE(u.HasValue() == true);
  REQUIRE(u.Value() == 2);
}
