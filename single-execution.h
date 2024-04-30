#pragma once
#include <optional>

template <typename F>
class SingleExecution
{
public:
  SingleExecution(F &&f): f_(std::move(f)) { }
  SingleExecution(const SingleExecution &) = delete;
  SingleExecution(SingleExecution &&se): f_(std::move(se.f_)) {
    se.f_ = std::nullopt;
  }
  SingleExecution &operator = (const SingleExecution &) = delete;
  SingleExecution &operator = (SingleExecution &&se) {
    (*this)();
    f_ = std::move(se.f_);
    se.f_ = std::nullopt;
    return *this;
  }
  ~SingleExecution() { (*this)(); }

  void operator () () {
    if (f_.has_value()) {
      F f = std::move(f_.value());
      f_ = std::nullopt;
      f();
    }
  }
private:
  std::optional<F> f_;
};
