#include "Collectable.h"
#include "CollectedHeap.h"

namespace GC {
  void Collectable::mark(size_t generation, bool mark_recent_only) {
    if (marked == generation || (mark_recent_only && this->generation != Generation::RecentlyAllocated))
      return;
    marked = generation;
    markChildren(generation, mark_recent_only);
  };

  void Collectable::forceMark(size_t generation, bool mark_recent_only) {
    marked = generation;
    markChildren(generation, mark_recent_only);
  }
}
