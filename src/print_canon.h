#ifndef PRINT_CANON
#define PRINT_CANON

#include <iostream>

#include "canon_ir.h"

class CanonIRPRinterVisitor : public CNodeVisitor {
 public:
  CanonIRPRinterVisitor();
  void Visit(CNode& n);
  void Visit(CPtrMov& n);
  void Visit(CAdd& n);
  void Visit(CMul& n);
  void Visit(CSet& n);
  void Visit(CInput& n);
  void Visit(COutput& n);
  void Visit(CLoop& n);

 private:
  void VisitNextCNode(CNode& n);
  void PrintWithIndent(const std::string& s);
  int _indent_level;
};

void PrintCanonIR(CNode* n);

#endif  // PRINT_CANON
