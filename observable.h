#pragma once
#include <utility>
#include <functional>
#include <iostream>
#include <concepts>

#include "./chain.h"

template <typename T> class Observable;

template <typename T>
concept observable = std::is_base_of_v<decltype(Observable(std::declval<T>())), T>;

template <typename T>
class Observable
{
public:
  using Observer = std::function<void(const T &, const T &)>;

  class Subject
  {
  public:
    Subject(const T &value): value_(value), obs_() {}

    Subject(const Subject &) = delete;
    Subject & operator = (const Subject &) = delete;

    Subject(Subject &&) = default;
    Subject & operator = (Subject &&) = default;

    template <typename F>
    typename Chain<Observer>::Deleter Observe(const F &ob) {
      return obs_.Add(std::function(ob));
    }

    const T &Value() const { return value_; }

    void Notify(const T &value) {
      obs_.ForEach([this, &value](const Observer &ob) { ob(value, value_); });
      value_ = value;
    }

  private:
    T value_;
    Chain<Observer> obs_;
  };

  template <typename U>
  class MapSubject: public Subject
  {
  public:
    template <typename F>
    MapSubject(Observable<U> ob, const F &func):
      Subject(func(ob.Value())),
      unob_(ob.Observe([this, func = func](const U &val, const U &valOld) {
        this->Notify(func(val));
      })) { }
  private:
    typename Observable<U>::Unobserve unob_;
  };

  class JoinSubject: public Subject
  {
  public:
    JoinSubject(Observable<Observable<T>> ob):
      Subject(ob.Value().Value()),
      unobInner_(ob.Value().Observe([this](const T &valNew, const T &valOld) {
        this->Notify(valNew);
      })),
      unobOutter_(ob.Observe([this](const Observable<T> &obNew, const Observable<T> &obOld) mutable {
        this->unobInner_ = std::move(obNew.Observe([this](const T &valNew, const T &valOld) {
          this->Notify(valNew);
        }));
        this->Notify(obNew.Value());
      })) { }

  private:
    typename Observable<T>::Unobserve unobInner_;
    typename Observable<Observable<T>>::Unobserve unobOutter_;
  };

  class Unobserve
  {
  public:
    Unobserve(std::shared_ptr<Subject> subject, Chain<Observer>::Deleter &&deleter)
      : subject_(subject), deleter_(std::move(deleter)) { }

    Unobserve(const Unobserve &) = delete;
    Unobserve& operator = (const Unobserve &) = delete;
    Unobserve(Unobserve &&) = default;
    Unobserve& operator = (Unobserve &&unob) = default;

    Observable<T> Run() {
      deleter_.Run();
      return Observable<T>(std::move(subject_));
    }
    
  private:
    std::shared_ptr<Subject> subject_;
    Chain<Observer>::Deleter deleter_;
  };

public:
  Observable(const T &value): Observable(std::make_shared<Subject>(value)) { }
  Observable(std::shared_ptr<Subject> subject) : subject_(subject) {}
  Observable(Observable<Observable<T>> ob) : Observable(std::make_shared<JoinSubject>(ob)) { }

  template <typename F>
  Unobserve Observe(const F &f) const {
    return Unobserve(subject_, subject_->Observe(f));
  }

  const T &Value() const {
    return subject_->Value();
  }

  template <typename F>
  Observable<std::invoke_result_t<F, T>> map(const F &f) const {
    using ResultType = Observable<std::invoke_result_t<F, T>>;
    return ResultType(std::make_shared<typename ResultType::MapSubject<T>>(*this, f));
  }

  template <typename F>
  std::invoke_result_t<F, T> bind(const F &f) const {
    using ResultType = std::invoke_result_t<F, T>;
    return ResultType(std::make_shared<typename ResultType::JoinSubject>(map(f)));
  }

  template <typename F>
  std::invoke_result_t<F, T> operator >> (const F &f) const requires observable<std::invoke_result_t<F, T>> {
    return this->bind(f);
  }

  template <typename F>
  Observable<std::invoke_result_t<F, T>> operator >> (const F &f) const {
    return this->map(f);
  }

protected:
  std::shared_ptr<Subject> subject_;
};

template <typename T>
class Mutable: public Observable<T>
{
public:
  Mutable(const T &value): Observable<T>(value) { }

  void Update(const T &value) {
    this->subject_->Notify(value);
  }
};
