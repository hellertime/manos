#ifndef SHELL_PARSER_H
#define SHELL_PARSER_H

/**
 * parser.h
 */

#include <torgo/dstring.h>

typedef struct ParseResult ParseResult;

void freeParseResult(ParseResult *);

typedef struct ParseToken ParseToken;
typedef struct ParseTokenIterator ParseTokenIterator;

void freeParseTokenIterator(ParseTokenIterator *it);
const String* getNextParseTokenIterator(ParseTokenIterator *it);

typedef struct InputChainIterator InputChainIterator;

void freeInputChainIterator(InputChainIterator *it);
const String* getNextInputChainIterator(InputChainIterator *it);

typedef struct Parser Parser;

Parser* mkParser(void);
void freeParser(Parser *p);

ParseTokenIterator* getParseTokenIteratorParser(Parser *p);
InputChainIterator* getInputChainIteratorParser(Parser *p);

const char* addInputParser(Parser *p, const char *input);

ParseResult* parseInputParser(Parser *parser);

int hasUnparsedInputParser(Parser *parser);

int isCompleteParseResult(const ParseResult *r);
int getLengthParseResult(const ParseResult *r);

ParseTokenIterator* getParseTokenIteratorParseResult(ParseResult *r);

#endif
