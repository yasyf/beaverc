#pragma once
#include <cstdio>

/*
This class keeps track of the garbage collected heap. The class must do all of the following:
  - provide an interface to allocate objects that will be supported by garbage collection.
  - keep track of all currently allocated objects.
  - keep track of the number of currently allocated objects.
  -
*/
class CollectedHeap {

public:

  /*
  The constructor should take as an argument the maximum size of the garbage collected heap.
  You get to decide what the units of this value should be. Your VM should compute
  this value based on the -mem parameter passed to it. Keep in mind, however, that
  your VM could be using some extra memory that is not managed by the garbage collector, so
  make sure you account for this.
  */
  CollectedHeap(int maxmem) {
  }


  /*
  return number of objects in the heap.
  This is different from the size of the heap, which should also be tracked
  by the garbage collector.
  */
  int getSize() {
    return size;
  }

  /*
  This method allocates an object of type T using the default constructor (with no parameters).
  T must be a subclass of Collectable. Before returning the object, it should be registered so that
  it can be deallocated later.
  */
  template<typename T>
  T* allocate() {
  }


  /*
  A variant of the method above; this version of allocate can be used to allocate objects whose constructor
  takes one parameter. Useful when allocating Integer or String objects.
  */
  template<typename T, typename ARG>
  T* allocate(ARG a) {
  }

  /*
  For performance reasons, you may want to implement specialized allocate methods to allocate particular kinds of objects.

  */




  /*
  This is the method that is called by the follow(...) method of a Collectable object. This
  is how a Collectable object lets the garbage collector know about other Collectable otjects pointed to
  by itself.
  */
  inline void markSuccessors(Collectable* next) {
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
  }




};

