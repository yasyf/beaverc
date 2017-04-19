#include "mem.h"

using namespace std;

size_t rss() {
  int ignored, resident;
  ifstream buffer("/proc/self/statm");
  buffer >> ignored >> resident;
  buffer.close();
  return resident * (sysconf(_SC_PAGE_SIZE) / 1024);
}
