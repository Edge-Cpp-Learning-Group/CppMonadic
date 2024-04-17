#pragma once
#include <memory>

class Effect {
public:
  virtual ~Effect() {};
  virtual void Run () = 0;
};

class OnceEffect: public Effect {
public:
  OnceEffect(std::unique_ptr<Effect> eff): eff_(std::move(eff)) {}
  OnceEffect(const OnceEffect &) = delete;
  OnceEffect(OnceEffect &&) = default;
  ~OnceEffect() { Run(); }

  OnceEffect& operator = (const OnceEffect &) = delete;
  OnceEffect& operator = (OnceEffect &&once) {
    Run();
    eff_ = std::move(once.eff_);
    return *this;
  }

  virtual void Run () override {
    if (eff_) {
      eff_->Run();
      eff_ = nullptr;
    }
  }

private:
  std::unique_ptr<Effect> eff_;
};
