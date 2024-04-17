#pragma once
#include <utility>
#include <functional>
#include <iostream>
#include "chain.h"

template <typename T>
class Observable
{
private:
  using Observer = std::function<void(const std::optional<T> &)>;

public:
  using Unobserve = decltype(std::declval<Chain<Observer>>().Add(std::declval<Observer>()));
  // using Unobserve = std::function<void()>;

public:
  Observable(): value_(std::nullopt), obs_() {}

  Observable(const Observable &) = delete;
  Observable & operator = (const Observable &) = delete;

  Observable(Observable &&) = default;
  Observable & operator = (Observable &&) = default;

  ~Observable() { Notify(std::nullopt); }

  template <typename F>
  Unobserve Observe(F &&ob) {
    ob(value_);
    return obs_.Add(std::function(std::forward<F>(ob)));
  }

protected:
  void Notify(std::optional<T> &&value) {
    value_ = std::move(value);
    obs_.ForEach([this](const Observer &ob) { ob(value_); });
  }

private:
  std::optional<T> value_;
  Chain<Observer> obs_;
};

template <typename T>
class PureObservable: public Observable<T>
{
public:
  PureObservable(T &&value): Observable<T>() {
    this->Notify(std::move(value));
  }
};

template <typename T>
class Subject: public PureObservable<T>
{
public:
  Subject(T &&value): PureObservable<T>(std::move(value)) { }

  void Update(T &&value) {
    this->Notify(std::move(value));
  }
};

template <typename T>
class MapObservable: public Observable<T>
{
public:
  template <typename U, typename F>
  MapObservable(Observable<U> &ob, F &&func):
    unob_(ob.Observe([this, func = std::move(func)](const std::optional<U> &val) {
      if (val.has_value()) {
        this->Notify(func(val.value()));
      } else {
        this->Notify(std::nullopt);
      }
    })) { }

private:
  SingleExecution unob_;
};

template <typename T, typename F>
Observable<std::invoke_result_t<F, T>> operator | (Observable<T> &ob, F &&f) {
  return MapObservable<std::invoke_result_t<F, T>>(ob, std::move(f));
}

  // fmap constructor
  // template <typename U, template <typename, typename...> class F>
  // Behavior(Behavior<U> &behU, F<T(U)> &&f) {

  // }

  // bind constructor
  // template <typename U, template <typename, typename...> class F>
  // Behavior(Behavior<U> &behU, F<Behavior<T>(U)> &&f) {
  // }