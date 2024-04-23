#pragma once
#include <optional>

template <typename T>
class Maybe
{
private:
  Maybe(const std::optional<T> &opt): opt_(opt) { }
public:
  Maybe(): Maybe(std::nullopt) { }
  Maybe(const T &val): Maybe(std::optional(val)) { }
  template <typename F, typename U>
  Maybe(const F f, const Maybe<U> &val): Maybe(val.HasValue() ? std::optional(f(val.Value())) : std::nullopt) { }
  Maybe(const Maybe<Maybe<T>> &mmVal): Maybe(mmVal.HasValue() ? mmVal.Value().opt_ : std::nullopt) { }

  const T &Value() const { return opt_.value(); }
  bool HasValue() const { return opt_.has_value(); }

private:
  std::optional<T> opt_;
};
