#pragma once
#include <utility>
#include <functional>
#include <memory>

#include "./chain.h"

template <typename T> class Observable;

template <typename T>
class Observable final
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

  class Updater
  {
  public:
    Updater(std::shared_ptr<Subject> subject): subject_(subject) { }
    void operator () (const T &val) const { subject_->Notify(val); }
  private:
    std::shared_ptr<Subject> subject_;
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

public:
  static std::pair<Observable<T>, Updater> Mutable(const T &value) {
    auto subject = std::make_shared<Subject>(value);
    return std::make_pair(Observable<T>(subject), Updater(subject));
  }
};

template <typename T>
class TransactionalUpdater
{
public:
  using Unobserve = Observable<T>::Unobserve;
  using Updater = Observable<T>::Updater;
public:
  TransactionalUpdater(Unobserve &&unob, Updater update):
    unob_(std::move(unob)), update_(update) { }

  template <typename F>
  void operator () (F &&f) {
    Observable<T> ob = unob_();
    f();
    unob_ = ob.Observe([this](const T &valNew, const T &) { this->update_(valNew); });
    update_(ob.Value());
  }

private:
  Updater update_;
  Unobserve unob_;
};

template <typename T>
std::pair<Observable<T>, TransactionalUpdater<T>> Transactional(Observable<T> ob) {
  auto [obT, updateT] = Observable<T>::Mutable(ob.Value());
  return std::make_pair(obT, TransactionalUpdater<T>(
    ob.Observe([=](const T &valNew, const T &) { updateT(valNew); }),
    updateT));
}
