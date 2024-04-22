#pragma once
#include <utility>
#include <functional>
#include <iostream>
#include <concepts>

#include "./chain.h"

class Effect {
public:
  virtual ~Effect() {};
  virtual void Run () = 0;
};

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

  class MapSubject: public Subject
  {
  public:
    template <typename U, typename F>
    MapSubject(const Observable<U> &ob, const F &func):
      Subject(func(ob.Value())),
      unob_(ob.Observe([this, func = func](const U &val, const U &valOld) {
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
    Unobserve(std::shared_ptr<Subject> subject, Chain<Observer>::Deleter &&deleter)
      : subject_(subject), deleter_(std::move(deleter)) { }

    virtual void Run() override {
      deleter_.Run();
      subject_ = nullptr;
    }
    
  private:
    std::shared_ptr<Subject> subject_;
    Chain<Observer>::Deleter deleter_;
  };

public:
  Observable(T &&value): Observable(std::make_shared<Subject>(std::forward<T>(value))) { }
  Observable(std::shared_ptr<Subject> subject) : subject_(subject) {}

  template <typename F>
  std::shared_ptr<Effect> Observe(const F &f) const {
    return std::make_shared<Unobserve>(subject_, subject_->Observe(f));
  }

  const T &Value() const {
    return subject_->Value();
  }

  template <typename F>
  Observable<std::invoke_result_t<F, T>> map(const F &f) const {
    using ResultType = Observable<std::invoke_result_t<F, T>>;
    return ResultType(std::make_shared<typename ResultType::MapSubject>(*this, f));
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
  Mutable(T &&value): Observable<T>(std::forward<T>(value)) { }

  void Update(T &&value) {
    this->subject_->Notify(std::forward<T>(value));
  }
};
