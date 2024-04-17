#pragma once
#include <memory>

class Effect {
public:
  virtual ~Effect() {};
  virtual void operator () () = 0;
};

class OnceEffect: public Effect {
public:
  OnceEffect(std::unique_ptr<Effect> eff): eff_(std::move(eff)) {}
  OnceEffect(const OnceEffect &) = delete;
  OnceEffect(OnceEffect &&) = default;
  ~OnceEffect() { (*this)(); }

  OnceEffect& operator = (const OnceEffect &) = delete;
  OnceEffect& operator = (OnceEffect &&once) {
    (*this)();
    eff_ = std::move(once.eff_);
    return *this;
  }

  virtual void operator () () override {
    if (eff_) {
      (*eff_)();
      eff_ = nullptr;
    }
  }

private:
  std::unique_ptr<Effect> eff_;
};
