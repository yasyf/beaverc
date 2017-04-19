#pragma once
#include <cstdio>
#include "CollectedHeap.fwd.h"

namespace GC {
  //Any object that inherits from collectable can be created and tracked by the garbage collector.
  class Collectable {
  public:
    Collectable(CollectedHeap& heap) : heap(heap) {}
    virtual ~Collectable() {}

    void mark();
  private:
    //Any private fields you add to the Collectable class will be accessible by the CollectedHeap
    //(since it is declared as friend below). You can think of these fields as the header for the object,
    //which will include metadata that is useful for the garbage collector.
  protected:
    /*
    The mark phase of the garbage collector needs to follow all pointers from the collectable objects, check
    if those objects have been marked, and if they have not, mark them and follow their pointers.
    The simplest way to implement this is to require that collectable objects implement a follow method
    that calls heap.markSuccessors( ) on all collectable objects that this object points to.
    markSuccessors() is the one responsible for checking if the object is marked and marking it.
    */
    CollectedHeap& heap;
    size_t marked = 0;

    friend CollectedHeap;

    virtual void markChildren() = 0;
    virtual size_t size() = 0;
  };
}
