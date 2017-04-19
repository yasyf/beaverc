#include "mem.h"

using namespace std;

size_t rss() {
  #ifdef MAC_OS
  return 1024; // Randomly selected for now to make things compile
  #else
  int ignored, resident;
  ifstream buffer("/proc/self/statm");
  buffer >> ignored >> resident;
  buffer.close();
  return resident * (sysconf(_SC_PAGE_SIZE) / 1024);
  #endif
}
