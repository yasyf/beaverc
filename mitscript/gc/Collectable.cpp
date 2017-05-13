#include "Collectable.h"
#include "CollectedHeap.h"

namespace GC {
  void Collectable::mark(size_t generation) {
    if (marked == generation)
      return;
    marked = generation;
    markChildren(generation);
  };
}
