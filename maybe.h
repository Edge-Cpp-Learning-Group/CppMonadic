#pragma once
#include <variant>
#include "monad.h"

template <typename T>
class Maybe {
private:
  std::variant<std::monostate, T> value_;
public:
  Maybe(const T &value): value_(value) {}
  Maybe(T && value): value_(value) {}
  Maybe(): value_() {};

  bool isJust() const { return value_.index() == 1; }
  bool isNothing() const { return value_.index() == 0; }

  template <typename U, template <typename, typename...> class F = std::function>
  U runMaybe(const U &dft, F<U(T)> f) const {
    return isJust() ? Maybe(std::get<T>(value_)) : dft;
  }

  template <typename U, template <typename, typename...> class F = std::function>
  Maybe<U> operator >>= (F<Maybe<U>(T)> &&f) const {
    return isJust() ? f(std::get<T>(value_)) : Maybe<U>();
  }
};
