#pragma once
#include <memory>
#include <iostream>

class IMission
{
public:
  virtual ~IMission() {}
  virtual void Run() const = 0;
public:
  template <typename F>
  static std::unique_ptr<IMission> Make(F &&f) {
    class Mission: public IMission
    {
    public:
      Mission(F &&f): f_(std::move(f)) {}
      virtual void Run() const override { f_(); }
    private:
      F f_;
    };

    return std::make_unique<Mission>(std::move(f));
  }
};

class OneTimeExecution
{
public:
  OneTimeExecution(std::unique_ptr<IMission> mission): mission_(std::move(mission)) {}
  OneTimeExecution(const OneTimeExecution &) = delete;
  OneTimeExecution(OneTimeExecution &&) = delete;
  OneTimeExecution& operator = (const OneTimeExecution &) = delete;
  OneTimeExecution& operator = (OneTimeExecution &&) = delete;
  ~OneTimeExecution() { (*this)(); }

  void operator () () {
    if (mission_) {
      std::move(mission_)->Run();
    }
  }

private:
  std::unique_ptr<IMission> mission_;
};

template <typename T>
class IHandler
{
public:
  virtual ~IHandler() {};
  virtual OneTimeExecution Invoke(const T &val) const = 0;

  template <typename F>
  static std::unique_ptr<IHandler> Make(F &&f) {
    class Handler: public IHandler<T>
    {
    public:
      Handler(const F &f):  f_(f) {}
      virtual OneTimeExecution Invoke(const T &val) const override {
        return OneTimeExecution(IMission::Make([f = f_, val = val]() {
          f(val);
        }));
      }
    private:
      F f_;
    };

    return std::make_unique<Handler>(std::move(f));
  }
};


template <typename T>
class ITask
{
public:
  virtual OneTimeExecution Execute(std::unique_ptr<IHandler<T>> handler) = 0;
  virtual ~ITask() {}
private:
  class PureTask: public ITask<T>
  {
  public:
    PureTask(const T &val): val_(val) {}
    virtual OneTimeExecution Execute(std::unique_ptr<IHandler<T>> handler) override {
      return OneTimeExecution(IMission::Make([handler = std::move(handler), val = val_](){
        return handler->Invoke(val);
      }));
    }
  private:
    T val_;
  };

  template <typename F, typename U>
  class MapTask: public ITask<T>
  {
  public:
    MapTask(const F &f, std::shared_ptr<ITask<U>> task): f_(f), task_(task) { }

    virtual OneTimeExecution Execute(std::unique_ptr<IHandler<T>> handler) override {
      return task_->Execute(IHandler<U>::Make([handler = std::move(handler), f = f_](const U &val) {
        handler->Invoke(f(val));
      }));
    }
  private:
    F f_;
    std::shared_ptr<ITask<U>> task_;
  };


public:
  template <typename F>
  OneTimeExecution Execute(const F &f) {
    return OneTimeExecution(Execute(IHandler<T>::Make(f)));
  }
public:
  static std::unique_ptr<ITask> Make(const T &val) {
    return std::make_unique<PureTask>(val);
  }

  template <typename F, typename U>
  static std::unique_ptr<ITask> Make(const F &f, std::shared_ptr<U> task) {
    return std::make_unique<MapTask<F, U>>(f, task);
  }
private:
  std::unique_ptr<ITask> impl_;
};
