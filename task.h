#pragma once
#include <memory>
#include <iostream>

class IMission
{
public:
  IMission(): called_(false) { }
  void operator () () {
    if (!called_) {
      called_ = true;
      Run();
    }
  }
protected:
  virtual void Run() const = 0;
private:
  bool called_;
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
      (*std::move(mission_))();
    }
  }

private:
  std::unique_ptr<IMission> mission_;
};

template <typename T>
class IHandler
{
public:
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

public:
  template <typename F>
  OneTimeExecution Execute(const F &f) {
    return OneTimeExecution(Execute(IHandler<T>::Make(f)));
  }
public:
  static std::unique_ptr<ITask> Make(const T &val) {
    return std::make_unique<PureTask>(val);
  }
private:
  std::unique_ptr<ITask> impl_;
};
