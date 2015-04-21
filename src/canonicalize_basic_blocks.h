#ifndef CANONICALIZE_BASIC_BLOCKS
#define CANONICALIZE_BASIC_BLOCKS

#include <stack>
#include <unordered_map>
#include <utility>

#include "canon_ir.h"

// Basic block: sequence with no inputs, outputs, or loops
struct BBInfo {
  // offset -> add amount
  std::unordered_map<int, int> additions;
  // op_offset -> (target_offset, multiplier)
  std::unordered_map<int, std::pair<int, int>> multiplications;
  int ptr_mov = 0;
};

class CanonicalizeVisitor : public CNodeVisitor {
 public:
  CanonicalizeVisitor();
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
  void AddSimpleStatement(CNode* n);
  void StartBB();
  void FinishBB();
  std::stack<CNode*> _blocks;
  BBInfo _current_bb = {};
  CNode* _start_node;
};

// Merge all adds, multiplies, and pointer movements
// Reorders instructions
CNode* CanonicalizeBasicBlocks(CNode* n);

#endif  // CANONICALIZE_BASIC_BLOCKS
