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
  Behavior(T &&value) : Behavior([value = std::forward<T>(value)]()
                             { return value; }) {}

  template <typename F>
  Behavior(F &&getVal) : getVal_(getVal), obs_() {}

  template <typename F>
  std::pair<T, Canceller> operator () (F &&ob)  {
    return std::make_pair<T, Canceller>(getVal_(), obs_.Add(std::function(std::forward<F>(ob))));
  }

  void Notify() {
    obs_.ForEach([](const Observer &ob) { ob(); });
    obs_.Clear();
  }

  // template <typename F>
  // Behavior<std::invoke_result_t<F, T>> fmap(F &&f) {
  //   using U = std::invoke_result_t<F, T>;

  //   return Behavior([])
  // }


private:
  std::function<T()> getVal_;
  Chain<std::function<void()>> obs_;
};
