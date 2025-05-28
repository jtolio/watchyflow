#pragma once

#include <memory>
#include <stdexcept>

// The idea behind this memory arena is that the Watchy is just going to
// tell its ESP32 hardware to deep sleep, and so who cares about freeing
// heap space? If we're going to use the heap, we might as well not worry
// about spending time deallocating and keeping the memory pool tidy.
// So this is a no-op deallocating memory arena.
class MemArena {
public:
  explicit MemArena(size_t size);

  MemArena(const MemArena &copy)        = delete;
  MemArena &operator=(const MemArena &) = delete;

  ~MemArena();

  void *allocate(size_t requested, size_t alignment);
  void deallocate(void *ptr, size_t size) noexcept;

  size_t used() { return current_ - begin_; }
  size_t remaining() { return end_ - current_; }

private:
  size_t size_;
  char *begin_;
  char *end_;
  char *current_;
};

extern MemArena globalArena;

// This makes it so the Memory Arena can be used for std containers.
template <typename T> class MemArenaAllocator {
public:
  using value_type = T;

  using propagate_on_container_copy_assignment = std::true_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap            = std::true_type;

  explicit MemArenaAllocator(MemArena &arena) noexcept : arena_(&arena) {}

  template <typename U>
  MemArenaAllocator(const MemArenaAllocator<U> &copy) noexcept
      : arena_(copy.arena_) {}

  T *allocate(size_t n) {
    return static_cast<T *>(arena_->allocate(n * sizeof(T), alignof(T)));
  }

  void deallocate(T *p, size_t n) noexcept {
    arena_->deallocate(p, n * sizeof(T));
  }

  friend bool operator==(const MemArenaAllocator &lhs,
                         const MemArenaAllocator &rhs) noexcept {
    return lhs.arena_ == rhs.arena_;
  }

  friend bool operator!=(const MemArenaAllocator &lhs,
                         const MemArenaAllocator &rhs) noexcept {
    return !(lhs == rhs);
  }

  template <typename> friend class MemArenaAllocator;

private:
  MemArena *arena_;
};
