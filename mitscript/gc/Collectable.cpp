#include "Collectable.h"
#include "CollectedHeap.h"

#define OVERHEAD_FACTOR 5

namespace GC {
  void Collectable::mark() {
    if (marked == heap.generation)
      return;
    this->marked = heap.generation;
    markChildren();
  };

  size_t Collectable::size() {
    return _size() * OVERHEAD_FACTOR;
  };
}
