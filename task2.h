#pragma once
#include "effect.h"

template <typename T>
class Task
{
private:
  class ITask
  {
  public:
    virtual ~ITask() = default;
    virtual Effect operator () () = 0;
  };

public:

  static constexpr Task Pure(const T &val) {
    // TODO
  }

  template <typename F, typename U>
  static constexpr Task Bind(const Task<U> &mVal, F &&f) {
    // TODO
  }

};
