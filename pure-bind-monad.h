#pragma once
#include <functional>

template <template <typename> typename M>
struct Monad
{
  template <typename T>
  static constexpr M<T> Pure(const T &val) {
    return M<T>::Pure(val);
  }

  template <typename F, typename T> 
  static constexpr std::invoke_result_t<F, T> Bind(const M<T> &mVal, F &&f) {
    return std::invoke_result_t<F, T>::Bind(mVal, std::forward<F>(f));
  }

  template <typename F, typename T>
  static constexpr M<std::invoke_result_t<F, T>> Map(F &&f, const M<T> &mVal) {
    return Bind(mVal, [f = std::move(f)](const T &val) { return Pure(f(val)); });
  }

  template <typename T>
  static constexpr M<T> Join(const M<M<T>> &mmVal) {
    return Bind(mmVal, [](const M<T> &mVal) { return mVal; });
  }

  template <typename F, typename V>
  static constexpr M<std::invoke_result_t<F, V>> Lift(F &&f, const M<V> &mVal) {
    return Map(std::move(f), mVal);
  }

  template <typename F, typename V, typename... Args>
  static constexpr auto Lift(F &&f, const M<V> &mVal, M<Args>... mArgs) {
    return Bind(mVal, [f = std::move(f), mArgs...](const V &v) {
      return Lift([f = std::move(f), v = v](const Args &... args) {
        return f(v, args...);
      }, mArgs...);
    });
  }
};
