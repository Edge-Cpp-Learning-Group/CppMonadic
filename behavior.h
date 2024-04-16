#pragma once
#include <utility>
#include <functional>
#include "chain.h"

template <typename T>
class Behavior
{
private:
  using Observer = std::function<void(const std::optional<T> &)>;

public:
  using Canceller = decltype(std::declval<Chain<Observer>>().Add(std::declval<Observer>()));

public:
  Behavior(T &&value) : Behavior([value = std::forward<T>(value)]()
                             { return value; }) {}

  template <typename F>
  Behavior(F &&getVal) : getVal_(getVal), obs_() {}

  ~Behavior() {
    Notify(getVal_());
  }

  template <typename F>
  std::pair<T, Canceller> operator () (F &&ob)  {
    return std::make_pair<T, Canceller>(getVal_(), obs_.Add(std::function(std::forward<F>(ob))));
  }

  template <typename F>
  Behavior<std::invoke_result_t<F, T>> fmap(F &&f) {
    using U = std::invoke_result_t<F, T>;

    return Behavior([
        &self = &this,
        cache = static_cast<std::optional<U>>(std::nullopt),
        f = std::forward<F>(f)
      ]() mutable {
      if (!cache.has_value()) {
        auto [valT, unob] = self([&cache, &f](std::optional<T> legacy) {
          if (legacy.has_value()) {
            cache = f(legacy.value());
          } else {
            cache = std::nullopt;
          }
        });
        cache = f(valT);
      }
      return cache.value();
    });
  }

protected:
  void Notify(const std::optional<T> &legacy) {
    obs_.ForEach([&legacy](const Observer &ob) { ob(legacy); });
    obs_.Clear();
  }

private:
  std::function<T()> getVal_;
  Chain<Observer> obs_;
};
