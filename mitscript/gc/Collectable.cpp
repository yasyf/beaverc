#include "Collectable.h"
#include "CollectedHeap.h"

#define SIZEOF_TO_BYTES 4
#define OVERHEAD_FACTOR 5

namespace GC {
  void Collectable::mark() {
    if (marked == heap.generation)
      return;
    this->marked = heap.generation;
    markChildren();
  };

  size_t Collectable::size() {
    return _size() * SIZEOF_TO_BYTES * OVERHEAD_FACTOR;
  };
}
