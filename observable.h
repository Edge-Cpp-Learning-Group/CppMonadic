#pragma once
#include <utility>
#include <functional>
#include <iostream>
#include "chain.h"

template <typename T>
class Observable
{
public:
  class Subject
  {
  private:
    using Observer = std::function<void(const T &, const T &)>;

  public:
    Subject(const T &value): value_(value), obs_() {}

    Subject(const Subject &) = delete;
    Subject & operator = (const Subject &) = delete;

    Subject(Subject &&) = default;
    Subject & operator = (Subject &&) = default;

    template <typename F>
    std::unique_ptr<Effect> Observe(F &&ob) {
      return obs_.Add(std::function(std::forward<F>(ob)));
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

  class MapSubject: public Subject
  {
  public:
    template <typename U, typename F>
    MapSubject(const Observable<U> &ob, F &&func):
      Subject(func(ob.Value())),
      unob_(ob.Observe([this, func = std::forward<F>(func)](const U &val, const U &valOld) {
        this->Notify(func(val));
      })) { }
  private:
    std::shared_ptr<Effect> unob_;
  };

  class JoinSubject: public Subject
  {
  public:
    JoinSubject(const Observable<Observable<T>> &ob): Subject(ob.Value().Value()) {
      ObserveInner(ob.Value());
      unobOutter_ = ob.Observe([this](const Observable<T> &obNew, const Observable<T> &obOld) mutable {
        this->Notify(obNew.Value());
        this->ObserveInner(obNew);
      });
    }
  private:
    void ObserveInner(const Observable<T> &ob) {
      this->unobInner_ = ob.Observe([this](const T &valueNew, const T &valueOld) {
        this->Notify(valueNew);
      });
    }
  private:
    std::shared_ptr<Effect> unobOutter_;
    std::shared_ptr<Effect> unobInner_;
  };

  class Unobserve : public Effect
  {
  public:
    Unobserve(std::shared_ptr<Subject> subject, std::unique_ptr<Effect> unob)
      : subject_(subject), unob_(std::move(unob)) { }

    virtual void Run() override {
      unob_ = nullptr;
      subject_ = nullptr;
    }
    
  private:
    std::shared_ptr<Subject> subject_;
    std::unique_ptr<Effect> unob_;
  };

public:
  Observable(T &&value): Observable(std::make_shared<Subject>(std::forward<T>(value))) { }
  Observable(std::shared_ptr<Subject> subject) : subject_(subject) {}

  template <typename F>
  std::shared_ptr<Effect> Observe(F &&f) const {
    return std::make_shared<Unobserve>(subject_, subject_->Observe(f));
  }

  const T &Value() const {
    return subject_->Value();
  }

  template <typename F>
  Observable<std::invoke_result_t<F, T>> map(F &&f) const {
    using ResultType = Observable<std::invoke_result_t<F, T>>;
    return ResultType(std::make_shared<ResultType::MapSubject>(*this, std::forward<F>(f)));
  }

  template <typename F>
  Observable<std::invoke_result_t<F, T>> operator | (F &&f) const {
    return this->map(std::forward<F>(f));
  }

  template <typename F>
  std::invoke_result_t<F, T> bind(F &&f) const {
    using ResultType = std::invoke_result_t<F, T>;
    return ResultType(std::make_shared<ResultType::JoinSubject>(*this | f));
  }

  template <typename F>
  std::invoke_result_t<F, T> operator >> (F &&f) const {
    return this->bind(std::forward<F>(f));
  }

protected:
  std::shared_ptr<Subject> subject_;
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
