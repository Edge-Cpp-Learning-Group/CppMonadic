#pragma once
#include <utility>
#include <functional>
#include <iostream>
#include "chain.h"

template <typename T>
class Subject
{
private:
  using Observer = std::function<void(const T &, const T &)>;

public:
  Subject(T &&value): value_(std::forward<T>(value)), obs_() {}

  Subject(const Subject &) = delete;
  Subject & operator = (const Subject &) = delete;

  Subject(Subject &&) = default;
  Subject & operator = (Subject &&) = default;

  template <typename F>
  std::unique_ptr<Effect> Observe(F &&ob) {
    return obs_.Add(std::function(std::forward<F>(ob)));
  }

  void Notify(T &&value) {
    obs_.ForEach([this, &value](const Observer &ob) { ob(value, value_); });
    value_ = std::forward<T>(value);
  }

  const T &Value() const { return value_; }

private:
  T value_;
  Chain<Observer> obs_;
};

template <typename T>
class Observable
{
public:
  class Unobserve : public Effect
  {
  public:
    Unobserve(std::shared_ptr<Subject<T>> subject, std::unique_ptr<Effect> unob)
      : subject_(subject), unob_(std::move(unob)) { }

    virtual void Run() override {
      unob_ = nullptr;
      subject_ = nullptr;
    }
    
  private:
    std::shared_ptr<Subject<T>> subject_;
    std::unique_ptr<Effect> unob_;
  };

public:
  Observable(T &&value) : subject_(std::make_shared<Subject<T>>(std::forward<T>(value))) {}

  template <typename F>
  std::shared_ptr<Effect> Observe(F &&f) const {
    return std::make_shared<Unobserve>(subject_, subject_->Observe(f));
  }

  const T &Value() const {
    return subject_->Value();
  }

protected:
  std::shared_ptr<Subject<T>> subject_;
};

template <typename T>
class PureObservable: public Observable<T>
{
public:
  PureObservable(T &&value): Observable<T>(std::forward<T>(value)) { }
};

template <typename T>
class Mutable: public Observable<T>
{
public:
  Mutable(T &&value): Observable<T>(std::forward<T>(value)) { }

  void Update(T &&value) {
    this->subject_->Notify(std::forward<T>(value));
  }
};

template <typename T>
class MapObservable: public Observable<T>
{
public:
  template <typename U, typename F>
  MapObservable(const Observable<U> &ob, F &&func):
    Observable<T>(func(ob.Value())),
    unob_(ob.Observe([this, func = std::move(func)](const U &val, const U &valOld) {
      this->subject_->Notify(func(val));
    })) { }

private:
  std::shared_ptr<Effect> unob_;
};

template <typename T, typename F>
MapObservable<std::invoke_result_t<F, T>> operator | (const Observable<T> &ob, F &&f) {
  return MapObservable<std::invoke_result_t<F, T>>(ob, std::move(f));
}
