#pragma once
#include <memory>
#include <optional>

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
        prev = nullptr;
      }
      if (next) {
        next->prev = prev;
        next = nullptr;
      }
      payload = std::nullopt;
    }
    Node *prev;
    Node *next;
    std::optional<T> payload;
  };

  // Methods
public:
  auto Add(T &&value)
  {
    std::unique_ptr<Node> node = std::make_unique<Node>();
    node->prev = head_.prev;
    node->next = &head_;
    node->payload = std::forward<T>(value);
    head_.prev = head_.prev->next = node.get();
    return SingleExecution([node = std::move(node)]() mutable { node = nullptr; });
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
