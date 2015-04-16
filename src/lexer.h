#ifndef LEXER
#define LEXER

#include <istream>

enum Token {
  END_OF_FILE = 0,
  INCR_PTR,
  DECR_PTR,
  INCR_DATA,
  DECR_DATA,
  INPUT_DATA,
  OUTPUT_DATA,
  START_LOOP,
  END_LOOP
};

Token GetNextToken(std::istream& input);

#endif  // LEXER
