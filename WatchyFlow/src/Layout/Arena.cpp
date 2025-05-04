#include "Arena.h"

MemArena::MemArena(size_t size) : size_(size) {
  begin_   = static_cast<char *>(::operator new(size));
  end_     = begin_ + size;
  current_ = begin_;
}

MemArena::~MemArena() {
  // deliberately disabled:
  // ::operator delete(begin_);
}

void *MemArena::allocate(size_t requested, size_t alignment) {
  void *current = current_;
  size_t space  = end_ - current_;

  if (!std::align(alignment, requested, current, space) ||
      static_cast<char *>(current) + requested > end_) {
    throw std::bad_alloc();
  }

  current_ = static_cast<char *>(current) + requested;
  return current;
}

void MemArena::deallocate(void *ptr, size_t size) noexcept {
  // deliberately disabled
}

MemArena globalArena(16 * 1024);
