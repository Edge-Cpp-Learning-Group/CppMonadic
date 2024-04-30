#pragma once

template <typename I>
struct Reader {
  template <typename T>
  class To
  {
  private:
    class ITo
    {
    public:
      virtual ~ITo() = default;
      virtual T operator() (const I &input) = 0;
    };

    template <typename F>
    class ToImpl: public ITo {
    public:
      ToImpl(F &&f): f_(std::forward<F>(f)) {}
      virtual T operator () (const I &input) override { return f_(input); }
    private:
      F f_;
    };

    std::shared_ptr<ITo> ptr_;

  public:
    template <typename F>
    To(F &&f):
      ptr_(new ToImpl<F>(std::forward<F>(f))) {}

    T operator () (const I &input) const { return (*ptr_)(input); }

    static constexpr To Pure(const T &val) {
      return To([val = val] (const I &) { return val; });
    }

    template <typename F, typename U>
    static constexpr To Bind(const To<U> &mVal, F &&f) {
      return To([mVal = mVal, f = std::move(f)](const I &input) {
        return f(mVal(input))(input);
      });
    }
  };
};