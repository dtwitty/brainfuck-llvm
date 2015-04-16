#include <istream>

#include "lexer.h"

Token GetNextToken(std::istream& input) {
  char tok;
  while (input.get(tok)) {
    switch (tok) {
      case '>':
        return INCR_PTR;
      case '<':
        return DECR_PTR;
      case '+':
        return INCR_DATA;
      case '-':
        return DECR_DATA;
      case ',':
        return INPUT_DATA;
      case '.':
        return OUTPUT_DATA;
      case '[':
        return START_LOOP;
      case ']':
        return END_LOOP;
      default:
        continue;
    }
  }
  return END_OF_FILE;
}
