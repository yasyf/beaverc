#pragma once
#include <cstdio>
#include "CollectedHeap.fwd.h"

namespace GC {
  enum class Generation {
    RecentlyAllocated,
    Old
  };

  //Any object that inherits from collectable can be created and tracked by the garbage collector.
  class Collectable {
  public:
    Collectable() {}

    void mark(size_t generation, bool mark_recent_only);
    void forceMark(size_t generation, bool mark_recent_only);
    virtual size_t size() = 0;

    Generation generation = Generation::RecentlyAllocated;
  private:
    virtual void markChildren(size_t generation, bool mark_recent_only) = 0;
  protected:
    /*
    The mark phase of the garbage collector needs to follow all pointers from the collectable objects, check
    if those objects have been marked, and if they have not, mark them and follow their pointers.
    The simplest way to implement this is to require that collectable objects implement a follow method
    that calls heap.markSuccessors( ) on all collectable objects that this object points to.
    markSuccessors() is the one responsible for checking if the object is marked and marking it.
    */
    size_t marked = 0;

    friend CollectedHeap;
  };
}
