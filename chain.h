#pragma once
#include <memory>
#include <optional>

template <typename T>
class Chain
{
// Types
private:
  class Node
  {
  public:
    Node(): prev_(this), next_(this), payload_(std::nullopt) { }
    Node(T &&value): prev_(this), next_(this), payload_(std::forward<T>(value)) { }

    Node(const Node &) = delete;
    Node & operator = (const Node &) = delete;

    Node(Node &&) = default;
    Node & operator = (Node &&) = default;

    ~Node() { Isolate(); }

    void Isolate() {
      prev_->next_ = next_;
      next_->prev_ = prev_;
      prev_ = this;
      next_ = this;
    }

    void InsertBefore(Node &node) {
      prev_ = node.prev_;
      next_ = &node;
      node.prev_->next_ = this;
      node.prev_ = this;
    }

    const Node *Next() const {
      return next_;
    }

    const T &Value() const {
      return payload_.value();
    }

  private:
    Node *prev_;
    Node *next_;
    std::optional<T> payload_;
  };

public:
  class Deleter
  {
  public:
    Deleter(std::unique_ptr<Node> ptr): ptr_(std::move(ptr)) {}

    Deleter(Deleter &&del) = default;
    Deleter& operator = (Deleter &&del) {
      (*this)();
      ptr_ = std::move(del.ptr_);
      return *this;
    }

    Deleter(const Deleter &) = delete;
    Deleter& operator = (const Deleter &) = delete;

    void operator () () { ptr_ = nullptr; }

  private:
    std::unique_ptr<Node> ptr_;
  };

  // Methods
public:
  Deleter Add(T &&value) {
    std::unique_ptr<Node> node = std::make_unique<Node>(std::forward<T>(value));
    node->InsertBefore(head_);
    return Deleter(std::move(node));
  }

  void Clear() {
    head_.Isolate();
  }

  template <typename F>
  void ForEach(F &&f) const {
    const Node *ptr = head_.Next();
    while (ptr != &head_) {
      f(ptr->Value());
      ptr = ptr->Next();
    }
  }

// Members
private:
  Node head_;
};
