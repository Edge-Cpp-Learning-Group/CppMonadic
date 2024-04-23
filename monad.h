#pragma once
#include <functional>
#include <type_traits>
#include <concepts>
#include <utility>

template <typename R, typename Arg, typename... Args>
std::function<R(Args...)> BindFn(std::function<R(Arg, Args...)> f, const Arg &arg) {
  return std::function([=](Args... args) { return f(arg, args...); });
}

template <template <typename> typename M>
struct Monad {
  template <typename T>
  static M<T> Pure(const T &val) {
    return M<T>(val);
  }

  template <typename F, typename T>
  static M<std::invoke_result_t<F, T>> Map(const F &f, M<T> mVal) {
    using ResultType = M<std::invoke_result_t<F, T>>;
    return ResultType(f, mVal);
  }

  template <typename F, typename T>
  static std::invoke_result_t<F, T> Bind(M<T> mVal, const F &f) {
    using U = std::invoke_result_t<F, T>;
    M<M<U>> mmVal = M<M<U>>(f, mVal);
    return M<U>(mmVal);
  }

  template <typename T>
  static M<T> Join(M<M<T>> mmVal) {
    return M<T>(mmVal);
  }

  template <typename F, typename V>
  static M<std::invoke_result_t<F, V>> Lift(const F &f, M<V> mVal) {
    return Monad<M>::Map(f, mVal);
  }

  template <typename F, typename V, typename... Args>
  static auto Lift(const F &f, M<V> mVal, Args... args) {
    return Bind(mVal, [=](const V &v) {
      return Lift(BindFn(std::function(f), v), args...);
    });
  }
};

template <template <typename> typename M, typename T>
concept monad = std::is_base_of_v<decltype(M(std::declval<T>())), T>;

template <template <typename> typename M, typename T, typename F>
std::invoke_result_t<F, T> operator >> (const M<T> &val, const F &f) requires monad<M, std::invoke_result_t<F, T>> {
  return Monad<M>::Bind(val, f);
}

template <template <typename> typename M, typename T, typename F>
M<std::invoke_result_t<F, T>> operator >> (const M<T> &val, const F &f) {
  return Monad<M>::Map(f, val);
}