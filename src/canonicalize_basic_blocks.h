#ifndef CANONICALIZE_BASIC_BLOCKS
#define CANONICALIZE_BASIC_BLOCKS

#include <stack>
#include <unordered_map>

#include "canon_ir.h"

// Basic block: sequence with only additions and ptr moves
struct BBInfo {
  // offset -> add amount
  std::unordered_map<int, int> additions;
  int ptr_mov = 0;
};

class CanonicalizeVisitor : public CNodeVisitor {
 public:
  CanonicalizeVisitor();
  void Visit(CNode* n);
  void Visit(CPtrMov* n);
  void Visit(CAdd* n);
  void Visit(CMul* n);
  void Visit(CSet* n);
  void Visit(CInput* n);
  void Visit(COutput* n);
  void Visit(CLoop* n);

  CNode* GetProgram() { return start_node_; }

 private:
  void VisitNextCNode(CNode* n);
  void AddSimpleStatement(CNode* n);
  void StartBB();
  void FinishBB();
  std::stack<CNode*> blocks_;
  BBInfo current_bb_ = {};
  CNode* start_node_;
};

// Merge all adds, multiplies, and pointer movements
// Reorders instructions
CNode* CanonicalizeBasicBlocks(CNode* n);

#endif  // CANONICALIZE_BASIC_BLOCKS
