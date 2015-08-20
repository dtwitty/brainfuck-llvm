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
  virtual void Visit(CNode* n) = 0;
  virtual void Visit(CPtrMov* n) = 0;
  virtual void Visit(CAdd* n) = 0;
  virtual void Visit(CMul* n) = 0;
  virtual void Visit(CSet* n) = 0;
  virtual void Visit(CInput* n) = 0;
  virtual void Visit(COutput* n) = 0;
  virtual void Visit(CLoop* n) = 0;
};

class CNode {
 public:
  virtual ~CNode() {}
  CNode* GetNextCNode() { return next_.get(); }
  void SetNextCNode(CNode* next) { next_.reset(next); }
  virtual void Accept(CNodeVisitor& visitor) { visitor.Visit(this); }

 private:
  std::unique_ptr<CNode> next_;
};

class CPtrMov : public CNode {
 public:
  CPtrMov() {}
  CPtrMov(int amt) { amt_ = amt; }
  void Accept(CNodeVisitor& visitor) { visitor.Visit(this); }
  int GetAmt() { return amt_; }
  void SetAmt(int amt) { amt_ = amt; }

 private:
  int amt_ = 0;
};

class CAdd : public CNode {
 public:
  CAdd() {}
  CAdd(int offset, int amt) {
    offset_ = offset;
    amt_ = amt;
  }
  void Accept(CNodeVisitor& visitor) { visitor.Visit(this); }
  int GetOffset() { return offset_; }
  int GetAmt() { return amt_; }
  void SetOffset(int offset) { offset_ = offset; }
  void SetAmt(int amt) { amt_ = amt; }

 private:
  int offset_ = 0;
  int amt_ = 0;
};

class CMul : public CNode {
 public:
  CMul() {}
  CMul(int op_offset, int target_offset, int amt) {
    op_offset_ = op_offset;
    target_offset_ = target_offset;
    amt_ = amt;
  }
  void Accept(CNodeVisitor& visitor) { visitor.Visit(this); }
  int GetOpOffset() { return op_offset_; }
  int GetTargetOffset() { return target_offset_; }
  int GetAmt() { return amt_; }
  void SetOpOffset(int offset) { op_offset_ = offset; }
  void SetTargetOffset(int offset) { target_offset_ = offset; }
  void SetAmt(int amt) { amt_ = amt; }

 private:
  int op_offset_ = 0;
  int target_offset_ = 0;
  int amt_ = 0;
};

class CSet : public CNode {
 public:
  CSet() {}
  CSet(int offset, int amt) {
    offset_ = offset;
    amt_ = amt;
  }
  void Accept(CNodeVisitor& visitor) { visitor.Visit(this); }
  int GetOffset() { return offset_; }
  int GetAmt() { return amt_; }
  void SetOffset(int offset) { offset_ = offset; }
  void SetAmt(int amt) { amt_ = amt; }

 private:
  int offset_ = 0;
  int amt_ = 0;
};

class CInput : public CNode {
 public:
  CInput() {}
  CInput(int offset) { offset_ = offset; }
  void Accept(CNodeVisitor& visitor) { visitor.Visit(this); }
  int GetOffset() { return offset_; }
  void SetOffset(int offset) { offset_ = offset; }

 private:
  int offset_ = 0;
};

class COutput : public CNode {
 public:
  COutput() {}
  COutput(int offset) { offset_ = offset; }
  void Accept(CNodeVisitor& visitor) { visitor.Visit(this); }
  int GetOffset() { return offset_; }
  void SetOffset(int offset) { offset_ = offset; }

 private:
  int offset_ = 0;
};

class CLoop : public CNode {
 public:
  CLoop() {}
  void Accept(CNodeVisitor& visitor) { visitor.Visit(this); }
  CNode* GetBody() { return body_.get(); }
  void SetBody(CNode* body) { body_.reset(body); }

 private:
  std::unique_ptr<CNode> body_;
};

#endif  // CANON_IR
