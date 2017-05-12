#pragma once
#include <iostream>

#ifdef DEBUG

template<typename T>
void debug(std::string prefix, T t) {
  std::cout << prefix << ": " << t << std::endl;
}

template<typename T>
void debug(T t) {
  std::cout << t << std::endl;
}

#else

#define debug(...)

#endif
