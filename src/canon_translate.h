#ifndef CANON_TRANSLATE
#define CANON_TRANSLATE

#include "parser.h"
#include "canon_ir.h"

class CanonTranslateVisitor : public ASTNodeVisitor {
 CanonTranslateVisitor();
  void Visit(ASTNode& s);
  void Visit(IncrPtr& s);
  void Visit(DecrPtr& s);
  void Visit(IncrData& s);
  void Visit(DecrData& s);
  void Visit(GetInput& s);
  void Visit(Output& s);
  void Visit(BFLoop& s);

  CNode* GetProgram();
};

CNode* TranslateASTToCanonIR(ASTNode* s);


#endif  // CANON_TRANSLATE
