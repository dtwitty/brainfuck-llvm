#ifndef CANON_IR
#define CANON_IR

#include "parser.h"

class CNode;
class CPtrMov;  // CPtrMov(x) -> ptr += x
class CAdd;     // CAdd(off,x) -> M[ptr+off] += x
class CMul;     // CMul(off,x,y) -> M[ptr+off+x] += M[ptr+off]*y
class CSet;     // CSet(off,x) -> M[ptr+off] = x
class CInput;   // CInput(off) -> M[ptr+off] = getchar()
class COutput;  // COutput(off) -> putchar(M[ptr+off])
class CLoop;    // CLoop(body) -> while(*ptr) {body}

class CNodeVisitor {
 public:
  virtual ~CNodeVisitor() {}
  virtual void Visit(CNode& n) = 0;
  virtual void Visit(CPtrMov& n) = 0;
  virtual void Visit(CAdd& n) = 0;
  virtual void Visit(CMul& n) = 0;
  virtual void Visit(CSet& n) = 0;
  virtual void Visit(CInput& n) = 0;
  virtual void Visit(COutput& n) = 0;
  virtual void Visit(CLoop& n) = 0;
};

class CNode {
 public:
  virtual ~CNode() {}
  CNode* GetNextCNode() { return _next.get(); }
  void SetNextCNode(CNode* next) { _next.reset(next); }
  virtual void Accept(CNodeVisitor& visitor) { visitor.Visit(*this); }

 private:
  std::unique_ptr<CNode> _next;
};

class CPtrMov : public CNode {
 public:
  CPtrMov() {}
  CPtrMov(int amt) { _amt = amt; }
  void Accept(CNodeVisitor& visitor) { visitor.Visit(*this); }
  int GetAmt() { return _amt; }
  void SetAmt(int amt) { _amt = amt; }

 private:
  int _amt = 0;
};

class CAdd : public CNode {
 public:
  CAdd() {}
  CAdd(int offset, int amt) {
    _offset = offset;
    _amt = amt;
  }
  void Accept(CNodeVisitor& visitor) { visitor.Visit(*this); }
  int GetOffset() { return _offset; }
  int GetAmt() { return _amt; }
  void SetOffset(int offset) { _offset = offset; }
  void SetAmt(int amt) { _amt = amt; }

 private:
  int _offset = 0;
  int _amt = 0;
};

class CMul : public CNode {
 public:
  CMul() {}
  CMul(int op_offset, int target_offset, int amt) {
    _op_offset = op_offset;
    _target_offset = target_offset;
    _amt = amt;
  }
  void Accept(CNodeVisitor& visitor) { visitor.Visit(*this); }
  int GetOpOffset() { return _op_offset; }
  int GetTargetOffset() { return _target_offset; }
  int GetAmt() { return _amt; }
  void SetOpOffset(int offset) { _op_offset = offset; }
  void SetTargetOffset(int offset) { _target_offset = offset; }
  void SetAmt(int amt) { _amt = amt; }

 private:
  int _op_offset = 0;
  int _target_offset = 0;
  int _amt = 0;
};

class CSet : public CNode {
 public:
  CSet() {}
  CSet(int offset, int amt) {
    _offset = offset;
    _amt = amt;
  }
  void Accept(CNodeVisitor& visitor) { visitor.Visit(*this); }
  int GetOffset() { return _offset; }
  int GetAmt() { return _amt; }
  void SetOffset(int offset) { _offset = offset; }
  void SetAmt(int amt) { _amt = amt; }

 private:
  int _offset = 0;
  int _amt = 0;
};

class CInput : public CNode {
 public:
  CInput() {}
  CInput(int offset) { _offset = offset; }
  void Accept(CNodeVisitor& visitor) { visitor.Visit(*this); }
  int GetOffset() { return _offset; }
  void SetOffset(int offset) { _offset = offset; }

 private:
  int _offset = 0;
};

class COutput : public CNode {
 public:
  COutput() {}
  COutput(int offset) { _offset = offset; }
  void Accept(CNodeVisitor& visitor) { visitor.Visit(*this); }
  int GetOffset() { return _offset; }
  void SetOffset(int offset) { _offset = offset; }

 private:
  int _offset = 0;
};

class CLoop : public CNode {
 public:
  CLoop() {}
  void Accept(CNodeVisitor& visitor) { visitor.Visit(*this); }
  CNode* GetBody() { return _body.get(); }
  void SetBody(CNode* body) { _body.reset(body); }

 private:
  std::unique_ptr<CNode> _body;
};

#endif  // CANON_IR
