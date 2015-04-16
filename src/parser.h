#ifndef PARSER
#define PARSER

#include <istream>

#include "lexer.h"

class Statement;
class IncrPtr;
class DecrPtr;
class IncrData;
class DecrData;
class GetInput;
class Output;
class BFLoop;

class StatementVisitor {
 public:
  virtual ~StatementVisitor() {}
  virtual void Visit(Statement& s) = 0;
  virtual void Visit(IncrPtr& s) = 0;
  virtual void Visit(DecrPtr& s) = 0;
  virtual void Visit(IncrData& s) = 0;
  virtual void Visit(DecrData& s) = 0;
  virtual void Visit(GetInput& s) = 0;
  virtual void Visit(Output& s) = 0;
  virtual void Visit(BFLoop& s) = 0;
};

class Statement {
 public:
  virtual ~Statement() {
    if (_next) delete _next;
  }

  Statement* GetNextStatement() { return _next; }

  void SetNextStatement(Statement* next) { _next = next; }

  virtual void Accept(StatementVisitor& Visitor) { Visitor.Visit(*this); }

 private:
  Statement* _next = NULL;
};

class IncrPtr : public Statement {
 public:
  IncrPtr() {}
  void Accept(StatementVisitor& Visitor) { Visitor.Visit(*this); }
};

class DecrPtr : public Statement {
 public:
  DecrPtr() {}
  void Accept(StatementVisitor& Visitor) { Visitor.Visit(*this); }
};

class IncrData : public Statement {
 public:
  IncrData() {}
  void Accept(StatementVisitor& Visitor) { Visitor.Visit(*this); }
};

class DecrData : public Statement {
 public:
  DecrData() {}
  void Accept(StatementVisitor& Visitor) { Visitor.Visit(*this); }
};

class GetInput : public Statement {
 public:
  GetInput() {}
  void Accept(StatementVisitor& Visitor) { Visitor.Visit(*this); }
};

class Output : public Statement {
 public:
  Output() {}
  void Accept(StatementVisitor& Visitor) { Visitor.Visit(*this); }
};

class BFLoop : public Statement {
 public:
  BFLoop() {}
  void Accept(StatementVisitor& Visitor) { Visitor.Visit(*this); }

  Statement* GetBody() { return _body; }

  void SetBody(Statement* body) { _body = body; }

 private:
  Statement* _body;
};

Statement* Parse(std::istream& input);

#endif  // PARSER
