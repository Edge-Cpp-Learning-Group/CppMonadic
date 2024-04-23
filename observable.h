#pragma once
#include <utility>
#include <functional>
#include <memory>

#include "./chain.h"

template <typename T> class Observable;

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
      if (value != value_) {
        obs_.ForEach([this, &value](const Observer &ob) { ob(value, value_); });
        value_ = value;
      }
    }

  private:
    T value_;
    Chain<Observer> obs_;
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

    Observable<T> operator () () {
      deleter_();
      return Observable<T>(std::move(subject_));
    }
    
  private:
    std::shared_ptr<Subject> subject_;
    Chain<Observer>::Deleter deleter_;
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
      unobOutter_(ob.Observe([this](const Observable<T> &obNew, const Observable<T> &obOld) {
        this->unobInner_ = std::move(obNew.Observe([this](const T &valNew, const T &valOld) {
          this->Notify(valNew);
        }));
        this->Notify(obNew.Value());
      })) { }

  private:
    typename Observable<T>::Unobserve unobInner_;
    typename Observable<Observable<T>>::Unobserve unobOutter_;
  };
private:
  Observable(std::shared_ptr<Subject> subject) : subject_(subject) {}

public:
  // Monadic
  // Functor::Map
  template <typename F, typename U>
  Observable(const F &f, const Observable<U> &val): Observable(std::make_shared<MapSubject<U>>(val, f)) { }
  // Monad::Pure
  Observable(const T &value): Observable(std::make_shared<Subject>(value)) { }
  // Monad::Join
  Observable(Observable<Observable<T>> ob) : Observable(std::make_shared<JoinSubject>(ob)) { }

  // Comparable
  bool operator == (const Observable<T> &ob) const {
    return subject_ == ob.subject_;
  }

  template <typename F>
  Unobserve Observe(const F &f) const {
    return Unobserve(subject_, subject_->Observe(f));
  }

  const T &Value() const {
    return subject_->Value();
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

template <typename T>
class Transactional: public Observable<T>
{
public:
  Transactional(Observable<T> ob):
    Observable<T>(ob.Value()),
    unob_(ob.Observe([this](const T &valNew, const T &valOld) { this->subject_->Notify(valNew); })) { }
  
  template <typename F>
  void Transaction(F &&f) {
    Observable<T> ob = unob_();
    f();
    unob_ = ob.Observe([this](const T &valNew, const T &valOld) { this->subject_->Notify(valNew); });
    this->subject_->Notify(ob.Value());
  }

private:
  typename Observable<T>::Unobserve unob_;
};
