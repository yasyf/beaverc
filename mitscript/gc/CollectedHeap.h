#pragma once
#include <list>
#include <cstdio>
#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>
#include "Collectable.h"
#include "../vm/options.h"

#define OVERHEAD_FACTOR 8

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
    list<Collectable*> allocated;

  private:
    template<typename T>
    void register_allocation(T* t) {
      if (has_option(OPTION_SHOW_MEMORY_TRACE)) {
        cout << "A," << (void*) t << endl;
      }
      allocated.push_back(t);
      increaseSize(sizeof(Collectable*));
    }

  public:
    size_t generation = 0;
    size_t max_bytes_used = 0;
    size_t bytes_max;
    size_t bytes_current = 0;

    /*
    The constructor should take as an argument the maximum size of the garbage collected heap.
    You get to decide what the units of this value should be. Your VM should compute
    this value based on the -mem parameter passed to it. Keep in mind, however, that
    your VM could be using some extra memory that is not managed by the garbage collector, so
    make sure you account for this.
    */
    CollectedHeap(size_t maxmem) : bytes_max(maxmem) {
      increaseSize(sizeof(CollectedHeap));
    }

    void increaseSize(size_t n) {
      #if DEBUG
        cout << "increasing stack by " << n << endl;
      #endif
      bytes_current += n * OVERHEAD_FACTOR;
      max_bytes_used = max(max_bytes_used, bytes_current);
    }

    void decreaseSize(size_t n) {
      #if DEBUG
        cout << "decreasing stack by " << n << endl;
      #endif
      bytes_current -= n * OVERHEAD_FACTOR;
    }

    /*
    return number of objects in the heap.
    This is different from the size of the heap, which should also be tracked
    by the garbage collector.
    */
    int getCount() {
      return allocated.size();
    }

    template<typename T, typename ARG>
    T* allocateUncollectable(ARG a) {
      return new T(*this, a);
    }

    /*
    This method allocates an object of type T using the default constructor (with no parameters).
    T must be a subclass of Collectable. Before returning the object, it should be registered so that
    it can be deallocated later.
    */
    template<typename T>
    T* allocate() {
      auto t = new T(*this);
      register_allocation(t);
      return t;
    }

    /*
    A variant of the method above; this version of allocate can be used to allocate objects whose constructor
    takes one parameter. Useful when allocating Integer or String objects.
    */
    template<typename T, typename ARG>
    T* allocate(ARG a) {
      auto t = new T(*this, a);
      register_allocation(t);
      return t;
    }

    template<typename T, typename ARG, typename ARG2>
    T* allocate(ARG a, ARG2 b) {
      auto t = new T(*this, a, b);
      register_allocation(t);
      return t;
    }

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

      for (auto c = begin; c != end; c++) {
        (*c)->mark();
      }

      auto it = allocated.begin();
      while (it != allocated.end()) {
        auto ptr = *it;
        if (ptr->marked != generation) {
          #ifdef DEBUG
            cout << "ABOUT TO COLLECT: ";
          #endif
          if (has_option(OPTION_SHOW_MEMORY_TRACE)) {
            cout << "D," << (void*) ptr << endl;
          }
          delete ptr;
          it = allocated.erase(it);
          decreaseSize(sizeof(Collectable*));
          continue;
        }

        ++it;
      }
    }
  };
}
