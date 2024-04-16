#pragma once
#include <functional>
#include <concepts>

template <template <typename> class M, typename A>
using constructor_return_t = decltype(M<A>{std::declval<A const &>()});

template <template <typename> class M, typename A, typename B, template <typename, typename...> class F>
using bind_return_t = decltype(std::declval<M<A> const &>() >>= std::declval<F<M<B>(A)>>());

struct DummyTypeA {};
struct DummyTypeB {};

template <template <typename> class M>
concept monad = requires {
  std::is_same_v<constructor_return_t<M, DummyTypeA>, M<DummyTypeA>>;
  std::is_same_v<bind_return_t<M, DummyTypeA, DummyTypeB, std::function>, M<DummyTypeB>>; 
};

template <template <typename> class M, typename T, typename U, template <typename, typename...> class F = std::function>
M<U> fmap(F<U(T)> &&f, M<T> x) requires monad<M> {
  return x >>= F<M<U>(T)>([_f = std::forward<F<U(T)>>(f)] (const T& val) {
    return M<decltype(_f(std::declval<T>()))>{_f(val)};
  });
}

