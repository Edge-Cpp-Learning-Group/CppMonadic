#include <catch2/catch_test_macros.hpp>
#include <vector>
#include "../chain.h"

template <typename T>
std::vector<T> toVector(const Chain<T> &chain) {
  std::vector<T> vec;
  chain.ForEach([&](const T &v) { vec.push_back(v); });
  return vec;
}

TEST_CASE("Add and remove elements", "[Chain]") {
  Chain<int> chain;

  REQUIRE(toVector(chain) == std::vector<int>{});

  auto rm1 = chain.Add(1);
  REQUIRE(toVector(chain) == std::vector<int>{1});

  // The objects are always add to the end of the chain.
  auto rm2 = chain.Add(2);
  REQUIRE(toVector(chain) == std::vector<int>{1, 2});

  do {
    auto rm3 = chain.Add(3);
    REQUIRE(toVector(chain) == std::vector<int>{1, 2, 3});
  } while (0);

  REQUIRE(toVector(chain) == std::vector<int>{1, 2});

  rm1.Run();
  REQUIRE(toVector(chain) == std::vector<int>{2});

  chain.Clear();
  REQUIRE(toVector(chain) == std::vector<int>{});

  // Note, the deleter is released immediately, the object is removed from the chain.
  chain.Add(4);
  REQUIRE(toVector(chain) == std::vector<int>{});
}

struct Tracker {
  Tracker(): moved(false) { count += 1; }

  Tracker(const Tracker &) = delete;
  Tracker & operator = (const Tracker &) = delete;

  Tracker(Tracker &&t) {
    moved = t.moved;
    t.moved = true;
  }
  Tracker & operator = (Tracker &&t) {
    if (!moved) {
      count -= 1;
    }
    moved = t.moved;
    t.moved = true;
  }

  ~Tracker() { if (!moved) { count -= 1; } }

  bool moved;

  static void Init() { count = 0; }
  static int count;
};

int Tracker::count = 0;


TEST_CASE("Memory management", "[Chain]") {
  Tracker::Init();
  Chain<Tracker> chain;

  REQUIRE(Tracker::count == 0);

  auto rm1 = chain.Add(Tracker{});
  REQUIRE(Tracker::count == 1);

  rm1.Run();
  REQUIRE(Tracker::count == 0);

  auto rm2 = chain.Add(Tracker{});
  REQUIRE(Tracker::count == 1);

  do {
    auto rm3 = chain.Add(Tracker{});
    REQUIRE(Tracker::count == 2);
  } while (0);

  REQUIRE(Tracker::count == 1);

  auto rm4 = chain.Add(Tracker{});
  REQUIRE(Tracker::count == 2);

  // Note, clearing the chain doesn't release the memory of the elements,
  // as well as the chain nodes holding the elements. The memory is owned
  // by the caller of Chain<T>::Add
  chain.Clear();
  REQUIRE(Tracker::count == 2);

  // Note, the deleter is released immediately, so the Tracker object is
  // released as well. The object is removed from the chain.
  chain.Add(Tracker{});
  REQUIRE(Tracker::count == 2);
}
