#pragma once
#include "effect.h"
#include "reader.h"
#include "single-execution.h"

template <typename T>
class Task
{
public:
  using UnitEffect = Effect<std::monostate>;
  using Callback = typename Reader<T>::To<UnitEffect>;
private:
  class ITask
  {
  public:
    virtual ~ITask() = default;
    virtual UnitEffect operator () (const Callback &cb) = 0;
  };

  template <typename F>
  class TaskImpl: public ITask
  {
  public:
    TaskImpl(F &&f): f_(std::forward<F>(f)) {}
    virtual UnitEffect operator () (const Callback &cb) override {
      return SingleExecution(f_(cb));
    }
  private:
    F f_;
  };

  std::shared_ptr<ITask> ptr_;
public:
  template <typename F>
  Task(F &&f):
    ptr_(new TaskImpl<F>(std::forward<F>(f))) { }
  
  UnitEffect operator () (const Callback &cb) {
    return (*ptr_)(cb);
  }

  static constexpr Task Pure(const T &val) {
    return Task([val = val](const Callback &cb) { return cb(val); });
  }

  template <typename F, typename U>
  static constexpr Task Bind(const Task<U> &mVal, F &&f) {
    return Task([mVal = mVal, f = std::move(f)](const Callback &cb) {
      return mVal([cb = cb, f = std::move(f)](const U &val) {
        return f(val)(cb);
      });
    });
  }
};
