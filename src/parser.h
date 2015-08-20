#ifndef PARSER
#define PARSER

#include <memory>

#include "lexer.h"

class ASTNode;
class IncrPtr;
class DecrPtr;
class IncrData;
class DecrData;
class GetInput;
class Output;
class BFLoop;

class ASTNodeVisitor {
 public:
  virtual ~ASTNodeVisitor() {}
  virtual void Visit(ASTNode* s) = 0;
  virtual void Visit(IncrPtr* s) = 0;
  virtual void Visit(DecrPtr* s) = 0;
  virtual void Visit(IncrData* s) = 0;
  virtual void Visit(DecrData* s) = 0;
  virtual void Visit(GetInput* s) = 0;
  virtual void Visit(Output* s) = 0;
  virtual void Visit(BFLoop* s) = 0;
};

class ASTNode {
 public:
  virtual ~ASTNode() {}
  ASTNode* GetNextASTNode() { return _next.get(); }
  void SetNextASTNode(ASTNode* next) { _next.reset(next); }
  virtual void Accept(ASTNodeVisitor& visitor) { visitor.Visit(this); }

 private:
  std::unique_ptr<ASTNode> _next;
};

class IncrPtr : public ASTNode {
 public:
  IncrPtr() {}
  void Accept(ASTNodeVisitor& visitor) { visitor.Visit(this); }
};

class DecrPtr : public ASTNode {
 public:
  DecrPtr() {}
  void Accept(ASTNodeVisitor& visitor) { visitor.Visit(this); }
};

class IncrData : public ASTNode {
 public:
  IncrData() {}
  void Accept(ASTNodeVisitor& visitor) { visitor.Visit(this); }
};

class DecrData : public ASTNode {
 public:
  DecrData() {}
  void Accept(ASTNodeVisitor& visitor) { visitor.Visit(this); }
};

class GetInput : public ASTNode {
 public:
  GetInput() {}
  void Accept(ASTNodeVisitor& visitor) { visitor.Visit(this); }
};

class Output : public ASTNode {
 public:
  Output() {}
  void Accept(ASTNodeVisitor& visitor) { visitor.Visit(this); }
};

class BFLoop : public ASTNode {
 public:
  BFLoop() {}
  void Accept(ASTNodeVisitor& visitor) { visitor.Visit(this); }
  ASTNode* GetBody() { return _body.get(); }
  void SetBody(ASTNode* body) { _body.reset(body); }

 private:
  std::unique_ptr<ASTNode> _body;
};

ASTNode* Parse(std::istream& input);

#endif  // PARSER
