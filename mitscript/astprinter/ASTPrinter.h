#pragma once

#include "../pprinter/PrettyPrinter.h"

using namespace std;

class ASTPrinter : public PrettyPrinter {
private:
  bool maybeExtraIndent();
  bool open(string name);
  void close(bool extra);
  bool start(string name);
  void end(bool extra);

public:
  void visit(Program& prog) override;
  void visit(Block& block) override;
  void visit(Assignment& assign) override;
  void visit(Call& call) override;
  void visit(Global& global) override;
  void visit(IfStatement& is) override;
  void visit(WhileLoop& wl) override;
  void visit(Return& ret) override;
  void visit(Function& func) override;
  void visit(Record& rec) override;
};
