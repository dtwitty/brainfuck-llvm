#ifndef ELIM_SIMPLE_LOOPS
#define ELIM_SIMPLE_LOOPS

#include <stack>
#include <unordered_map>

#include "canon_ir.h"

// A simple loop is defined as having only data additions
// and a net pointer movement of zero
// These can be merged into multiplications followed by a set
class SimpleLoopElimVisitor : public CNodeVisitor {
 public:
  SimpleLoopElimVisitor();
  void Visit(CNode& n);
  void Visit(CPtrMov& n);
  void Visit(CAdd& n);
  void Visit(CMul& n);
  void Visit(CSet& n);
  void Visit(CInput& n);
  void Visit(COutput& n);
  void Visit(CLoop& n);

  CNode* GetProgram() { return _start_node; }

 private:
  void VisitNextCNode(CNode& n);
  void AddSimpleStatement(CNode* n);
  void StartSimpleLoop();
  std::stack<CNode*> _blocks;
  CNode* _start_node;
  bool _is_simple;
  std::unordered_map<int, int> _mult_map;
  int _ptr_mov;
};

CNode* EliminateSimpleLoops(CNode* n);

#endif  // ELIM_SIMPLE_LOOPS
