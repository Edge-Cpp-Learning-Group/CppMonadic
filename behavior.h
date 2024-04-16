#pragma once
#include <utility>
#include "chain.h"

template <typename T>
class Behavior
{
private:
  using Observer = std::function<void()>;
public:
  using Canceller = decltype(std::declval<Chain<Observer>>().Add(std::declval<Observer>()));

public:
  Behavior(T &&value) : getVal_([value = std::forward<T>(value)]()
                                { return value; }),
                        obs_() {}

  template <typename F>
  std::pair<T, Canceller> operator () (F &&ob)  {
    return std::make_pair<T, Canceller>(getVal_(), obs_.Add(std::function(std::forward<F>(ob))));
  }

  void Notify() {
    obs_.ForEach([](const Observer &ob) { ob(); });
    obs_.Clear();
  }

private:
  std::function<T()> getVal_;
  Chain<std::function<void()>> obs_;
};
