#pragma once
#include <memory>

class IMission
{
public:
  IMission(): called_(false) { }
  IMission(const IMission &) = delete;
  IMission(IMission &&) = delete;
  virtual ~IMission() final { (*this)(); }
  IMission& operator = (const IMission &) = delete;
  IMission& operator = (IMission &&) = delete;
  void operator () () {
    if (!called_) {
      called_ = true;
      Run();
    }
  }
protected:
  virtual void Run() = 0;
private:
  bool called_;
public:
  template <typename F>
  static std::unique_ptr<IMission> Make(F &&f) {
    class Mission: public IMission
    {
    public:
      Mission(F &&f): f_(std::move(f)) {}
      virtual void Run override { f(); }
    private:
      F f_;
    };

    return std::make_unique<Mission>(std::move(f));
  }
};

template <typename T>
class IHandler
{
public:
  virtual std::unique_ptr<IMission> Invoke(const T &val) const = 0;

  template <typename F>
  static std::unique_ptr<IHandler> Make(F &&f) {
    class Handler: public IHandler<T>
    {
    public:
      Handler(const F &f):  f_(f) {}
      virtual std::unique_ptr<IMission> Invoke(const T &val) override {
        return IMission::Make([f = f_, val = val]() { f(val); });
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
  virtual std::unique_ptr<IMission> Execute(std::unique_ptr<IHandler> handler) = 0;
private:
  class PureTask: public ITask<T>
  {
  public:
    PureTask(const T &val): val_(val) {}
    virtual std::unique_ptr<IMission> Execute(std::unique_ptr<IHandler> handler) override {
      return IMission::Make([=](){ return handler->Invoke(val_); });
    }
  private:
    T val_;
  };

public:
  template <typename F>
  std::unique_ptr<IMission> Execute(const F &f) {
    return Execute(IHandler::Make(f));
  }
public:
  static std::unique_ptr<ITask> Make(const T &val) {
    return std::make_unique<PureTask>(val);
  }
private:
  std::unique_ptr<ITask> impl_;
};
