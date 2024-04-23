#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <utility>
#include <memory>
#include <format>

#include "../observable.h"
#include "../monad.h"

template <typename T>
class MockObserver {
public:
  MockObserver(): calls_(new std::vector<std::pair<T, T>>()) {}

  void operator () (const T &newVal, const T &oldVal) {
    calls_->push_back(std::pair<T, T>{ newVal, oldVal });
  }

  int callCount() const {
    return calls_->size();
  }

  std::pair<T, T> nthCallArgs(int i) const {
    return calls_->at(i);
  }

  std::pair<T, T> lastCallArgs() const {
    return nthCallArgs(callCount() - 1);
  }

private:
  std::shared_ptr<std::vector<std::pair<T, T>>> calls_;
};

TEST_CASE("Observe and unobserve", "[Observable]") {
  MockObserver<int> ob;

  auto [x, UpdateX] = Observable<int>::Mutable(0);

  auto unob1 = x.Observe(ob);
  REQUIRE(ob.callCount() == 0);

  UpdateX(1);
  REQUIRE(ob.callCount() == 1);
  REQUIRE(ob.lastCallArgs() == std::pair{1, 0});

  unob1();

  UpdateX(2);
  REQUIRE(ob.callCount() == 1);

  do {
    auto unob2 = x.Observe(ob);
    UpdateX(3);
    REQUIRE(ob.callCount() == 2);
    REQUIRE(ob.lastCallArgs() == std::pair{3, 2});
  } while (0);

  UpdateX(4);
  REQUIRE(ob.callCount() == 2);

  auto unob3 = x.Observe(ob);
  UpdateX(4);
  REQUIRE(ob.callCount() == 2);
}

TEST_CASE("Method map as Functor", "[Observable]") {
  MockObserver<int> obI;
  MockObserver<std::string> obS;

  auto [x, UpdateX] = Observable<int>::Mutable(0);
  Observable<std::string> obsStr = Monad<Observable>::Map([](int i) { return std::to_string(i); }, x);

  auto unobI = x.Observe(obI);
  auto unobS = obsStr.Observe(obS);

  UpdateX(1);

  REQUIRE(obI.callCount() == 1);
  REQUIRE(obI.lastCallArgs() == std::pair{1, 0});
  REQUIRE(obS.callCount() == 1);
  REQUIRE(obS.lastCallArgs() == std::pair{"1", "0"});
}

TEST_CASE("Method pure (constructor) as Monad", "[Observable]") {
  Observable<int> obs(0);
  REQUIRE(obs.Value() == 0);
}

TEST_CASE("Method bind as Monad", "[Observable]") {
  MockObserver<std::string> ob;
  auto [x, UpdateX] = Observable<int>::Mutable(0);
  auto [y, UpdateY] = Observable<int>::Mutable(1);
  auto [z, UpdateZ] = Observable<int>::Mutable(2);

  auto formular = Monad<Observable>::Bind(x, [=](int x) {
    return Monad<Observable>::Bind(y, [=](int y) {
      return Monad<Observable>::Map([=](int z) {
        int w = (x + y) * z;
        return std::format("({} + {}) * {} = {}", x, y, z, w);
      }, z);
    });
  });


  REQUIRE(formular.Value() == "(0 + 1) * 2 = 2");

  auto unob = formular.Observe(ob);

  UpdateX(3);
  REQUIRE(ob.callCount() == 1);
  REQUIRE(ob.lastCallArgs() == std::pair{"(3 + 1) * 2 = 8", "(0 + 1) * 2 = 2"});

  UpdateY(4);
  REQUIRE(ob.callCount() == 2);
  REQUIRE(ob.lastCallArgs() == std::pair{"(3 + 4) * 2 = 14", "(3 + 1) * 2 = 8"});

  UpdateZ(5);
  REQUIRE(ob.callCount() == 3);
  REQUIRE(ob.lastCallArgs() == std::pair{"(3 + 4) * 5 = 35", "(3 + 4) * 2 = 14"});
}

TEST_CASE("Method join as Monad", "[Observable]") {
  auto [x, UpdateX] = Observable<int>::Mutable(0);
  auto [y, UpdateY] = Observable<int>::Mutable(1);
  auto [z, UpdateZ] = Observable<Observable<int>>::Mutable(x);
  Observable<int> w(z);
  MockObserver<int> ob;

  auto unob = w.Observe(ob);

  REQUIRE(w.Value() == 0);

  UpdateX(2);
  REQUIRE(w.Value() == 2);
  REQUIRE(ob.callCount() == 1);
  REQUIRE(ob.lastCallArgs() == std::pair(2, 0));

  UpdateZ(y);
  REQUIRE(w.Value() == 1);
  REQUIRE(ob.callCount() == 2);
  REQUIRE(ob.lastCallArgs() == std::pair(1, 2));

  UpdateY(3);
  REQUIRE(w.Value() == 3);
  REQUIRE(ob.callCount() == 3);
  REQUIRE(ob.lastCallArgs() == std::pair(3, 1));

  UpdateX(4);
  REQUIRE(w.Value() == 3);
  REQUIRE(ob.callCount() == 3);
}

TEST_CASE("Sugar with operator >>", "[Observable]") {
  MockObserver<std::string> ob;
  auto [x, UpdateX] = Observable<int>::Mutable(0);
  auto [y, UpdateY] = Observable<int>::Mutable(1);
  auto [z, UpdateZ] = Observable<int>::Mutable(2);

  auto formular =
    x >> [=](int x) { return
    y >> [=](int y) { return
    z >> [=](int z) {
      int w = (x + y) * z;
      return std::format("({} + {}) * {} = {}", x, y, z, w);
    }; }; };


  REQUIRE(formular.Value() == "(0 + 1) * 2 = 2");

  auto unob = formular.Observe(ob);

  UpdateX(3);
  REQUIRE(ob.callCount() == 1);
  REQUIRE(ob.lastCallArgs() == std::pair{"(3 + 1) * 2 = 8", "(0 + 1) * 2 = 2"});

  UpdateY(4);
  REQUIRE(ob.callCount() == 2);
  REQUIRE(ob.lastCallArgs() == std::pair{"(3 + 4) * 2 = 14", "(3 + 1) * 2 = 8"});

  UpdateZ(5);
  REQUIRE(ob.callCount() == 3);
  REQUIRE(ob.lastCallArgs() == std::pair{"(3 + 4) * 5 = 35", "(3 + 4) * 2 = 14"});
}

TEST_CASE("Transactional update", "[Transactional]") {
  MockObserver<std::string> ob;
  auto [x, UpdateX] = Observable<int>::Mutable(0);
  auto [y, UpdateY] = Observable<int>::Mutable(1);
  auto [z, UpdateZ] = Observable<int>::Mutable(2);

  auto formular =
    static_cast<Observable<int>>(x) >> [=](int x) { return
    static_cast<Observable<int>>(y) >> [=](int y) { return
    static_cast<Observable<int>>(z) >> [=](int z) {
      int w = (x + y) * z;
      return std::format("({} + {}) * {} = {}", x, y, z, w);
    }; }; };

  auto [obs, Transaction] = Transactional(formular);

  REQUIRE(obs.Value() == "(0 + 1) * 2 = 2");

  auto unob = obs.Observe(ob);

  Transaction([&]() {
    UpdateX(3);
    UpdateY(4);
    UpdateZ(5);
  });

  REQUIRE(ob.callCount() == 1);
  REQUIRE(ob.lastCallArgs() == std::pair{"(3 + 4) * 5 = 35", "(0 + 1) * 2 = 2"});
}

TEST_CASE("Lift Calls", "[Observable]") {
  MockObserver<int> ob;
  auto [x, UpdateX] = Observable<int>::Mutable(0);
  auto [y, UpdateY] = Observable<int>::Mutable(1);
  auto [z, UpdateZ] = Observable<int>::Mutable(2);

  auto obs = Monad<Observable>::Lift([](int a, int b, int c) {
    return a + b + c;
  }, x, y, z);

  REQUIRE(obs.Value() == 3);

  auto unob = obs.Observe(ob);

  UpdateX(3);
  REQUIRE(obs.Value() == 6);
  REQUIRE(ob.callCount() == 1);
  REQUIRE(ob.lastCallArgs() == std::pair{6, 3});

  UpdateY(4);
  REQUIRE(obs.Value() == 9);
  REQUIRE(ob.callCount() == 2);
  REQUIRE(ob.lastCallArgs() == std::pair{9, 6});

  UpdateZ(5);
  REQUIRE(obs.Value() == 12);
  REQUIRE(ob.callCount() == 3);
  REQUIRE(ob.lastCallArgs() == std::pair{12, 9});
}

TEST_CASE("Integrated test case", "[Observable]") {
  MockObserver<int> ob;
  auto [x, UpdateX] = Observable<int>::Mutable(0);
  auto [y, UpdateY] = Observable<int>::Mutable(1);
  auto [z, UpdateZ] = Observable<int>::Mutable(2);
  auto obs0 = Monad<Observable>::Lift([](int a, int b, int c) {
    return a + b + c;
  }, x, y, z);

  auto [obs, Transaction] = Transactional(
    Monad<Observable>::Lift([](int a, int b, int c) {
      return a + b + c;
    }, x, y, z)
  );

  auto unob = obs.Observe(ob);

  Transaction([=]() mutable {
    UpdateX(3);
    UpdateY(4);
    UpdateZ(5);
  });

  REQUIRE(obs.Value() == 12);
  REQUIRE(ob.callCount() == 1);
  REQUIRE(ob.lastCallArgs() == std::pair{12, 3});

  Transaction([=]() mutable {
    UpdateX(5);
    UpdateY(4);
    UpdateZ(3);
  });
  REQUIRE(obs.Value() == 12);
  REQUIRE(ob.callCount() == 1);
}
