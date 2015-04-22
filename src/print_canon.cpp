#include <iostream>
#include <sstream>

#include "canon_ir.h"
#include "print_canon.h"

CanonIRPRinterVisitor::CanonIRPRinterVisitor() { _indent_level = 0; }

void CanonIRPRinterVisitor::VisitNextCNode(CNode& n) {
  CNode* next = n.GetNextCNode();
  if (next) {
    next->Accept(*this);
  }
}

void CanonIRPRinterVisitor::PrintWithIndent(const std::string& s) {
  for (int i = 0; i < _indent_level; i++) {
    std::cerr << "  ";
  }
  std::cerr << s << std::endl;
}

void CanonIRPRinterVisitor::Visit(CNode& n) {
  PrintWithIndent("CNode");
  VisitNextCNode(n);
}

void CanonIRPRinterVisitor::Visit(CPtrMov& n) {
  int amt = n.GetAmt();
  std::stringstream ss;
  ss << "CPtrMov(" << amt << ")";
  PrintWithIndent(ss.str());
  VisitNextCNode(n);
}

void CanonIRPRinterVisitor::Visit(CAdd& n) {
  int amt = n.GetAmt();
  int offset = n.GetOffset();
  std::stringstream ss;
  ss << "CAdd(" << offset << "," << amt << ")";
  PrintWithIndent(ss.str());
  VisitNextCNode(n);
}

void CanonIRPRinterVisitor::Visit(CMul& n) {
  int amt = n.GetAmt();
  int op_offset = n.GetOpOffset();
  int target_offset = n.GetTargetOffset();
  std::stringstream ss;
  ss << "CMul(" << op_offset << "," << target_offset << "," << amt << ")";
  PrintWithIndent(ss.str());
  VisitNextCNode(n);
}

void CanonIRPRinterVisitor::Visit(CSet& n) {
  int amt = n.GetAmt();
  int offset = n.GetOffset();
  std::stringstream ss;
  ss << "CSet(" << offset << "," << amt << ")";
  PrintWithIndent(ss.str());
  VisitNextCNode(n);
}

void CanonIRPRinterVisitor::Visit(CInput& n) {
  int offset = n.GetOffset();
  std::stringstream ss;
  ss << "CInput(" << offset << ")";
  PrintWithIndent(ss.str());
  VisitNextCNode(n);
}

void CanonIRPRinterVisitor::Visit(COutput& n) {
  int offset = n.GetOffset();
  std::stringstream ss;
  ss << "COutput(" << offset << ")";
  PrintWithIndent(ss.str());
  VisitNextCNode(n);
}

void CanonIRPRinterVisitor::Visit(CLoop& n) {
  PrintWithIndent("CLoop:");
  _indent_level += 1;
  n.GetBody()->Accept(*this);
  _indent_level -= 1;
  VisitNextCNode(n);
}

void PrintCanonIR(CNode* n) {
  CanonIRPRinterVisitor visitor;
  if (n) {
    n->Accept(visitor);
  }
}
