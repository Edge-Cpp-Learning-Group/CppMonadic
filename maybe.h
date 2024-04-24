#pragma once
#include <optional>

template <typename T>
class Maybe
{
private:
  Maybe(const std::optional<T> &opt): opt_(opt) { }
public:
  Maybe(): Maybe(std::nullopt) { }
  // Monad::Pure
  Maybe(const T &val): Maybe(std::optional(val)) { }
  // Functor::Map
  template <typename F, typename U>
  Maybe(const F f, const Maybe<U> &val): Maybe(val.HasValue() ? std::optional(f(val.Value())) : std::nullopt) { }
  // Monad::Join
  Maybe(const Maybe<Maybe<T>> &mmVal): Maybe(mmVal.HasValue() ? mmVal.Value().opt_ : std::nullopt) { }

  const T &Value() const { return opt_.value(); }

  bool HasValue() const { return opt_.has_value(); }

  bool operator == (const Maybe &mVal) const { return opt_ == mVal.opt_; }

  explicit operator bool () const { return opt_.has_value(); }

private:
  std::optional<T> opt_;
};
