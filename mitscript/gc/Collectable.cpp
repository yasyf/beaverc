#include "Collectable.h"
#include "CollectedHeap.h"

namespace GC {
  void Collectable::mark(uint32_t generation, bool mark_recent_only) {
    if (marked == generation || (mark_recent_only && this->is_old))
      return;
    marked = generation;
    markChildren(generation, mark_recent_only);
  };

  void Collectable::forceMark(uint32_t generation, bool mark_recent_only) {
    marked = generation;
    markChildren(generation, mark_recent_only);
  }
}
