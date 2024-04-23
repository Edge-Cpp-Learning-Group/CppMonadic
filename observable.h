#pragma once
#include <utility>
#include <functional>
#include <concepts>

#include "./chain.h"

template <typename T> class Observable;

template <typename T>
concept observable = std::is_base_of_v<decltype(Observable(std::declval<T>())), T>;

template <typename R, typename Arg, typename... Args>
std::function<R(Args...)> BindFn(std::function<R(Arg, Args...)> f, const Arg &arg) {
  return std::function([=](Args... args) { return f(arg, args...); });
}

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
  // Functor::Map
  template <typename F, typename U>
  Observable(const F &f, const Observable<U> &val): Observable(std::make_shared<MapSubject<U>>(val, f)) { }
  // Monad::Pure
  Observable(const T &value): Observable(std::make_shared<Subject>(value)) { }
  // Monad::Join
  Observable(Observable<Observable<T>> ob) : Observable(std::make_shared<JoinSubject>(ob)) { }

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

  template <typename F>
  Observable<std::invoke_result_t<F, T>> Map(const F &f) const {
    using ResultType = Observable<std::invoke_result_t<F, T>>;
    return ResultType(f, *this);
  }

  template <typename F>
  std::invoke_result_t<F, T> Bind(const F &f) const {
    using ResultType = std::invoke_result_t<F, T>;
    return ResultType(Map(f));
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

template <template <typename> typename M>
struct Monad {
  template <typename T>
  static M<T> Pure(const T &val) {
    return M<T>(val);
  }

  template <typename F, typename T>
  static M<std::invoke_result_t<F, T>> Map(const F &f, M<T> mVal) {
    return mVal.Map(f);
  }

  template <typename F, typename T>
  static std::invoke_result_t<F, T> Bind(const F &f, M<T> mVal) {
    return mVal.Bind(f);
  }

  template <typename T>
  static M<T> Join(M<M<T>> mmVal) {
    return M<T>(mmVal);
  }

  template <typename F, typename V>
  static M<std::invoke_result_t<F, V>> Lift(const F &f, M<V> ob) {
    return ob.Map(f);
  }

  template <typename F, typename V, typename... Args>
  static auto Lift(const F &f, M<V> ob, Args... args) {
    return ob.Bind([=](const V &v) {
      return Lift(BindFn(std::function(f), v), args...);
    });
  }
};

template <template <typename> typename M, typename T>
concept monad = std::is_base_of_v<decltype(M(std::declval<T>())), T>;

template <template <typename> typename M, typename T, typename F>
std::invoke_result_t<F, T> operator >> (const M<T> &val, const F &f) requires monad<M, std::invoke_result_t<F, T>> {
  return val.Bind(f);
}

template <template <typename> typename M, typename T, typename F>
M<std::invoke_result_t<F, T>> operator >> (const M<T> &val, const F &f) {
  return val.Map(f);
}
