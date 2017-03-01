#pragma once

#include "PrettyPrinter.h"

using namespace std;

class ASTPrinter : public PrettyPrinter {
private:
  bool maybeExtraIndent();
  bool open(string name);
  void close(bool extra);
  bool start(string name);
  void end(bool extra);
};
