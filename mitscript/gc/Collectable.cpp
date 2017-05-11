#include "Collectable.h"
#include "CollectedHeap.h"

namespace GC {
  void Collectable::mark() {
    if (marked == heap.generation)
      return;
    this->marked = heap.generation;
    markChildren();
  };
}
