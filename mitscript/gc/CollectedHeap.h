#pragma once
#include <cstdio>
#include <vector>
#include <memory>
#include "Collectable.h"

using namespace std;

namespace GC {
  /*
  This class keeps track of the garbage collected heap. The class must do all of the following:
    - provide an interface to allocate objects that will be supported by garbage collection.
    - keep track of all currently allocated objects.
    - keep track of the number of currently allocated objects.
    -
  */
  class CollectedHeap {
    vector<weak_ptr<Collectable>> allocated;

  private:

    template<typename T>
    shared_ptr<T> _allocate(T* t_) {
      shared_ptr<T> t = shared_ptr<T>(t_);
      allocated.push_back(t);
      return t;
    }

  public:
    size_t generation = 0;
    size_t bytes_max;
    volatile size_t bytes_current = 0;

    /*
    The constructor should take as an argument the maximum size of the garbage collected heap.
    You get to decide what the units of this value should be. Your VM should compute
    this value based on the -mem parameter passed to it. Keep in mind, however, that
    your VM could be using some extra memory that is not managed by the garbage collector, so
    make sure you account for this.
    */
    CollectedHeap(size_t maxmem) : bytes_max(maxmem) {}

    void increaseSize(size_t n) {
      __sync_fetch_and_add(&bytes_current, n);
    }

    void decreaseSize(size_t n) {
      __sync_fetch_and_sub(&bytes_current, n);
    }

    /*
    return number of objects in the heap.
    This is different from the size of the heap, which should also be tracked
    by the garbage collector.
    */
    int getCount() {
      return allocated.size();
    }

    /*
    This method allocates an object of type T using the default constructor (with no parameters).
    T must be a subclass of Collectable. Before returning the object, it should be registered so that
    it can be deallocated later.
    */
    template<typename T>
    shared_ptr<T> allocate() {
      return _allocate(new T(*this));
    }

    /*
    A variant of the method above; this version of allocate can be used to allocate objects whose constructor
    takes one parameter. Useful when allocating Integer or String objects.
    */
    template<typename T, typename ARG>
    shared_ptr<T> allocate(ARG a) {
      return _allocate(new T(*this, a));
    }

    template<typename T, typename ARG, typename ARG2>
    shared_ptr<T> allocate(ARG a, ARG2 b) {
      return _allocate(new T(*this, a, b));
    }

    /*
    For performance reasons, you may want to implement specialized allocate methods to allocate particular kinds of objects.

    */

    /*
    The gc method should be called by your VM (or by other methods in CollectedHeap)
    whenever the VM decides it is time to reclaim memory. This method
    triggers the mark and sweep process.

    The ITERATOR type should support comparison, assignment and the ++ operator.
    I should also be able to dereference an interator to get a Collectable object.
    This code will take iterators marking the [begin, end) range of the rootset

    */
    template<typename ITERATOR>
    void gc(ITERATOR begin, ITERATOR end) {
      generation++;

      for (auto c = begin; c != end; c++)
        (*c)->mark();

      auto it = allocated.begin();
      while (it != allocated.end()) {
        if (it->expired()) {
          it = allocated.erase(it);
          continue;
        }

        auto lit = it->lock();
        if (lit->marked != generation) {
          delete lit.get();
          it = allocated.erase(it);
          continue;
        }

        ++it;
      }
    }
  };
}
