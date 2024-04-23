#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <utility>
#include <memory>
#include <format>

#include "../observable.h"

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

  Mutable<int> mut(0);

  auto unob1 = mut.Observe(ob);
  REQUIRE(ob.callCount() == 0);

  mut.Update(1);
  REQUIRE(ob.callCount() == 1);
  REQUIRE(ob.lastCallArgs() == std::pair{1, 0});

  unob1();

  mut.Update(2);
  REQUIRE(ob.callCount() == 1);

  do {
    auto unob2 = mut.Observe(ob);
    mut.Update(3);
    REQUIRE(ob.callCount() == 2);
    REQUIRE(ob.lastCallArgs() == std::pair{3, 2});
  } while (0);

  mut.Update(4);
  REQUIRE(ob.callCount() == 2);

  auto unob3 = mut.Observe(ob);
  mut.Update(4);
  REQUIRE(ob.callCount() == 2);
}

TEST_CASE("Method map as Functor", "[Observable]") {
  MockObserver<int> obI;
  MockObserver<std::string> obS;

  Mutable<int> mut(0);
  Observable<std::string> obsStr = mut.Map([](int i) { return std::to_string(i); });

  auto unobI = mut.Observe(obI);
  auto unobS = obsStr.Observe(obS);

  mut.Update(1);

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
  Mutable<int> x(0);
  Mutable<int> y(1);
  Mutable<int> z(2);

  auto obsFormular = Monad<Observable>::Bind([=](int x) {
    return Monad<Observable>::Bind([=](int y) {
      return z.Map([=](int z) {
        int w = (x + y) * z;
        return std::format("({} + {}) * {} = {}", x, y, z, w);
      });
    }, y);
  }, x);


  REQUIRE(obsFormular.Value() == "(0 + 1) * 2 = 2");

  auto unob = obsFormular.Observe(ob);

  x.Update(3);
  REQUIRE(ob.callCount() == 1);
  REQUIRE(ob.lastCallArgs() == std::pair{"(3 + 1) * 2 = 8", "(0 + 1) * 2 = 2"});

  y.Update(4);
  REQUIRE(ob.callCount() == 2);
  REQUIRE(ob.lastCallArgs() == std::pair{"(3 + 4) * 2 = 14", "(3 + 1) * 2 = 8"});

  z.Update(5);
  REQUIRE(ob.callCount() == 3);
  REQUIRE(ob.lastCallArgs() == std::pair{"(3 + 4) * 5 = 35", "(3 + 4) * 2 = 14"});
}

TEST_CASE("Method join as Monad", "[Observable]") {
  Mutable<int> x(0);
  Mutable<int> y(1);
  Mutable<Observable<int>> z(x);
  Observable<int> w(z);
  MockObserver<int> ob;

  auto unob = w.Observe(ob);

  REQUIRE(w.Value() == 0);

  x.Update(2);
  REQUIRE(w.Value() == 2);
  REQUIRE(ob.callCount() == 1);
  REQUIRE(ob.lastCallArgs() == std::pair(2, 0));

  z.Update(y);
  REQUIRE(w.Value() == 1);
  REQUIRE(ob.callCount() == 2);
  REQUIRE(ob.lastCallArgs() == std::pair(1, 2));

  y.Update(3);
  REQUIRE(w.Value() == 3);
  REQUIRE(ob.callCount() == 3);
  REQUIRE(ob.lastCallArgs() == std::pair(3, 1));

  x.Update(4);
  REQUIRE(w.Value() == 3);
  REQUIRE(ob.callCount() == 3);
}

TEST_CASE("Sugar with operator >>", "[Observable]") {
  MockObserver<std::string> ob;
  Mutable<int> x(0);
  Mutable<int> y(1);
  Mutable<int> z(2);

  auto obsFormular =
    static_cast<Observable<int>>(x) >> [=](int x) { return
    static_cast<Observable<int>>(y) >> [=](int y) { return
    static_cast<Observable<int>>(z) >> [=](int z) {
      int w = (x + y) * z;
      return std::format("({} + {}) * {} = {}", x, y, z, w);
    }; }; };


  REQUIRE(obsFormular.Value() == "(0 + 1) * 2 = 2");

  auto unob = obsFormular.Observe(ob);

  x.Update(3);
  REQUIRE(ob.callCount() == 1);
  REQUIRE(ob.lastCallArgs() == std::pair{"(3 + 1) * 2 = 8", "(0 + 1) * 2 = 2"});

  y.Update(4);
  REQUIRE(ob.callCount() == 2);
  REQUIRE(ob.lastCallArgs() == std::pair{"(3 + 4) * 2 = 14", "(3 + 1) * 2 = 8"});

  z.Update(5);
  REQUIRE(ob.callCount() == 3);
  REQUIRE(ob.lastCallArgs() == std::pair{"(3 + 4) * 5 = 35", "(3 + 4) * 2 = 14"});
}

TEST_CASE("Transactional update", "[Transactional]") {
  MockObserver<std::string> ob;
  Mutable<int> x(0);
  Mutable<int> y(1);
  Mutable<int> z(2);

  auto obsFormular =
    static_cast<Observable<int>>(x) >> [=](int x) { return
    static_cast<Observable<int>>(y) >> [=](int y) { return
    static_cast<Observable<int>>(z) >> [=](int z) {
      int w = (x + y) * z;
      return std::format("({} + {}) * {} = {}", x, y, z, w);
    }; }; };

  Transactional<std::string> obTrans(obsFormular);

  REQUIRE(obTrans.Value() == "(0 + 1) * 2 = 2");

  auto unob = obTrans.Observe(ob);

  obTrans.Transaction([&]() {
    x.Update(3);
    y.Update(4);
    z.Update(5);
  });

  REQUIRE(ob.callCount() == 1);
  REQUIRE(ob.lastCallArgs() == std::pair{"(3 + 4) * 5 = 35", "(0 + 1) * 2 = 2"});
}

TEST_CASE("Lift Calls", "[Observable]") {
  MockObserver<int> ob;
  Mutable<int> x(0);
  Mutable<int> y(1);
  Mutable<int> z(2);

  auto obs = Monad<Observable>::Lift([](int a, int b, int c) {
    return a + b + c;
  }, x, y, z);

  REQUIRE(obs.Value() == 3);

  auto unob = obs.Observe(ob);

  x.Update(3);
  REQUIRE(obs.Value() == 6);
  REQUIRE(ob.callCount() == 1);
  REQUIRE(ob.lastCallArgs() == std::pair{6, 3});

  y.Update(4);
  REQUIRE(obs.Value() == 9);
  REQUIRE(ob.callCount() == 2);
  REQUIRE(ob.lastCallArgs() == std::pair{9, 6});

  z.Update(5);
  REQUIRE(obs.Value() == 12);
  REQUIRE(ob.callCount() == 3);
  REQUIRE(ob.lastCallArgs() == std::pair{12, 9});
}

TEST_CASE("Integrated test case", "[Observable]") {
  MockObserver<int> ob;
  Mutable<int> x(0);
  Mutable<int> y(1);
  Mutable<int> z(2);
  auto obs0 = Monad<Observable>::Lift([](int a, int b, int c) {
    return a + b + c;
  }, x, y, z);

  auto obs = Transactional(
    // Observable<int>([](int a, int b, int c) {
    //   return a + b + c;
    // }, x, y, z)
    Monad<Observable>::Lift([](int a, int b, int c) {
      return a + b + c;
    }, x, y, z)
  );

  auto unob = obs.Observe(ob);

  obs.Transaction([=]() mutable {
    x.Update(3);
    y.Update(4);
    z.Update(5);
  });

  REQUIRE(obs.Value() == 12);
  REQUIRE(ob.callCount() == 1);
  REQUIRE(ob.lastCallArgs() == std::pair{12, 3});

  obs.Transaction([=]() mutable {
    x.Update(5);
    y.Update(4);
    z.Update(3);
  });
  REQUIRE(obs.Value() == 12);
  REQUIRE(ob.callCount() == 1);
}
