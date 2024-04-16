#pragma once
#include <memory>
#include <optional>
#include <iostream>

template <typename F>
class SingleExecution {
public:
  SingleExecution(F &&f): f_(std::forward<F>(f)), callable_(true) { }
  SingleExecution(const SingleExecution<F> &) = delete;
  SingleExecution(SingleExecution<F> &&se) : f_(std::move(se.f_)), callable_(se.callable_)
  {
    se.callable_ = false;
  }
  ~SingleExecution() { (*this)(); }

  SingleExecution<F>& operator = (SingleExecution &&se) {
    (*this)();
    f_ = std::forward<F>(se.f_);
    callable_ = se.callable_;
    se.callable_ = false;
    return *this;
  }

  void operator () () {
    if (callable_) {
      callable_ = false;
      f_();
    }
  }
private:
  F f_;
  bool callable_;
};

template <typename T>
class Chain
{
// Types
private:
  struct Node
  {
    Node(): prev(this), next(this), payload(std::nullopt) { }
    ~Node()
    {
      if (prev) {
        prev->next = next;
      }
      if (next) {
        next->prev = prev;
      }
      prev = nullptr;
      next = nullptr;
      payload = std::nullopt;
    }
    Node *prev;
    Node *next;
    std::optional<T> payload;
  };

  class Deleter
  {
  public:
    Deleter(std::unique_ptr<Node> ptr): ptr_(std::move(ptr)) {}
    Deleter(Deleter &&del) = default;
    Deleter(const Deleter &) = delete;
    Deleter& operator = (Deleter &&del) = default;
    Deleter& operator = (const Deleter &) = delete;

    void operator () () { ptr_ = nullptr; }

  private:
    std::unique_ptr<Node> ptr_;
  };

  // Methods
public:
  auto Add(T &&value)
  {
    std::unique_ptr<Node> node = std::make_unique<Node>();
    node->prev = head_.prev;
    node->next = &head_;
    node->payload = std::forward<T>(value);
    head_.prev->next = node.get();
    head_.prev = node.get();
    return SingleExecution(Deleter(std::move(node)));
  }

  void Clear() {
    head_.prev->next = head_.next;
    head_.next->prev = head_.prev;
    head_.prev = &head_;
    head_.next = &head_;
  }

  template <typename F>
  void ForEach(F &&f) const {
    Node *ptr = head_.next;
    while (ptr != &head_) {
      f(static_cast<const T &>(ptr->payload.value()));
      ptr = ptr->next;
    }
  }

// Members
private:
  Node head_;
};

template <typename T>
void printChain(const Chain<T> &chain) {
    chain.ForEach([](const T &value) {
        std::cout << value << ", ";
    });
    std::cout << std::endl;
}

