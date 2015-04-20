#include <istream>
#include <stack>
#include <iostream>

#include "lexer.h"
#include "parser.h"

void ParserError(const std::string& error) {
  std::cerr << error << std::endl;
  exit(-1);
}

ASTNode* Parse(std::istream& input) {
  Token tok;
  ASTNode* next, *loop_body, *first = new ASTNode();
  BFLoop* next_loop;
  std::stack<ASTNode*> blocks;
  blocks.push(first);

  while ((tok = GetNextToken(input))) {
    ASTNode* to_append = blocks.top();
    switch (tok) {
      case INCR_PTR:
        next = new IncrPtr();
        to_append->SetNextASTNode(next);
        blocks.top() = next;
        break;
      case DECR_PTR:
        next = new DecrPtr();
        to_append->SetNextASTNode(next);
        blocks.top() = next;
        break;
      case INCR_DATA:
        next = new IncrData();
        to_append->SetNextASTNode(next);
        blocks.top() = next;
        break;
      case DECR_DATA:
        next = new DecrData();
        to_append->SetNextASTNode(next);
        blocks.top() = next;
        break;
      case INPUT_DATA:
        next = new GetInput();
        to_append->SetNextASTNode(next);
        blocks.top() = next;
        break;
      case OUTPUT_DATA:
        next = new Output();
        to_append->SetNextASTNode(next);
        blocks.top() = next;
        break;
      case START_LOOP:
        next_loop = new BFLoop();
        to_append->SetNextASTNode(next_loop);
        blocks.top() = next_loop;
        loop_body = new ASTNode();
        blocks.push(loop_body);
        next_loop->SetBody(loop_body);
        break;
      case END_LOOP:
        if (blocks.size() > 1) {
          blocks.pop();
        } else {
          ParserError("Unmatched end-loop");
          return NULL;
        }
        break;
      default:
        ParserError("Unknown token type (should never happen)");
        return NULL;
    }
  }
  if (blocks.size() > 1) {
    ParserError("Unmatched start-loop");
    return NULL;
  }

  return first;
}
