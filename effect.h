#pragma once
#include <memory>
#include <variant>

template <typename F>
concept returning_void = std::is_same_v<std::invoke_result_t<F>, void>;

template <typename T = std::monostate>
class Effect final
{
private:
  class IEffect {
  public:
    virtual ~IEffect() {}
    virtual T operator () () = 0;
  };

  template <typename F>
  class EffectImpl: public IEffect {
  public:
    EffectImpl(F &&f): f_(std::forward<F>(f)) {}
    virtual T operator() () override { return f_(); }
  private:
    F f_;
  };

  std::shared_ptr<IEffect> ptr_;

public:
  template <typename F>
  Effect(F &&f):
    ptr_(std::make_unique<EffectImpl<F>>(std::forward<F>(f))) {}
  
  template <returning_void F>
  Effect(F &&f):
    ptr_(new EffectImpl([f = std::move(f)] () mutable { f(); return T(); })) {}

  T operator () () const { return (*ptr_)(); }

  static constexpr Effect Pure(const T &val) {
    return Effect([val] { return val; });
  }

  template <typename F, typename U>
  static constexpr Effect Bind(const Effect<U> &mVal, F &&f) {
    return Effect([mVal = mVal, f = std::move(f)] { return f(mVal())(); });
  }
};
