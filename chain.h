#pragma once
#include <memory>
#include <optional>
#include <iostream>
#include "effect.h"

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

  class Deleter: public Effect
  {
  public:
    Deleter(std::unique_ptr<Node> ptr): ptr_(std::move(ptr)) {}

    // Deleter(Deleter &&del) = default;
    // Deleter& operator = (Deleter &&del) = default;

    // Deleter(const Deleter &) = delete;
    // Deleter& operator = (const Deleter &) = delete;

    virtual void Run () override { ptr_ = nullptr; }

  private:
    std::unique_ptr<Node> ptr_;
  };

  // Methods
public:
  std::unique_ptr<Effect> Add(T &&value)
  {
    std::unique_ptr<Node> node = std::make_unique<Node>();
    node->prev = head_.prev;
    node->next = &head_;
    node->payload = std::forward<T>(value);
    head_.prev->next = node.get();
    head_.prev = node.get();
    return std::make_unique<OnceEffect>(
        std::make_unique<Deleter>(std::move(node)));
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

