#include <catch2/catch_test_macros.hpp>
#include "../monad.h"
#include "../maybe.h"

TEST_CASE("Map as Functor", "[Maybe]") {
  auto x = Maybe<int>(0);
  auto y = Maybe<int>();

  auto z = Monad<Maybe>::Map([](int n) { return n + 1; }, x);

  REQUIRE(z.HasValue() == true);
  REQUIRE(z.Value() == 1);

  auto w = Monad<Maybe>::Map([](int n) { return n + 1; }, y);
  REQUIRE(w.HasValue() == false);
}

TEST_CASE("Bind as Monad", "[Maybe]") {
  auto x = Maybe<int>(0);
  auto y = Maybe<int>();

  auto z = Monad<Maybe>::Bind(x, [](int n) { return Maybe(n + 1); });

  REQUIRE(z.HasValue() == true);
  REQUIRE(z.Value() == 1);

  auto w = Monad<Maybe>::Bind(y, [](int n) { return Maybe(n + 1); });
  REQUIRE(w.HasValue() == false);

  auto u = Monad<Maybe>::Bind(x, [](int n) { return Maybe<int>(); });
  REQUIRE(u.HasValue() == false);
}

TEST_CASE("Join as Monad", "[Maybe]") {
  auto x = Monad<Maybe>::Join(Maybe<Maybe<int>>(Maybe(1)));
  REQUIRE(x.HasValue() == true);
  REQUIRE(x.Value() == 1);

  auto y = Monad<Maybe>::Join(Maybe<Maybe<int>>(Maybe<int>()));
  REQUIRE(y.HasValue() == false);

  auto z = Monad<Maybe>::Join(Maybe<Maybe<int>>());
  REQUIRE(z.HasValue() == false);
}

TEST_CASE("Sugar with operator >>", "[Maybe]") {
  auto x = Maybe<int>(1);

  auto y = x >> [](int n) { return n + 1; };
  REQUIRE(y.HasValue() == true);
  REQUIRE(y.Value() == 2);

  auto z = y >> [](int n) { return Maybe(n * 2); }
             >> [](int n) { return n + 1; };
  
  REQUIRE(z.HasValue() == true);
  REQUIRE(z.Value() == 5);

  auto w = z >> [](int n) { return Maybe(n * 2); }
             >> [](int) { return Maybe<int>(); };

  REQUIRE(w.HasValue() == false);
}

TEST_CASE("Lift Calls", "[Maybe]") {
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
