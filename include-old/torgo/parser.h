#ifndef SHELL_PARSER_H
#define SHELL_PARSER_H

/**
 * parser.h
 */

#include <torgo/string.h>

struct ParseResult;

void freeParseResult(struct ParseResult *);

struct ParseToken;
struct ParseTokenIterator;

void freeParseTokenIterator(struct ParseTokenIterator *it);
const struct String* getNextParseTokenIterator(struct ParseTokenIterator *it);

struct InputChainIterator;

void freeInputChainIterator(struct InputChainIterator *it);
const struct String* getNextInputChainIterator(struct InputChainIterator *it);

struct Parser;

struct Parser* mkParser(void);
void freeParser(struct Parser *p);

struct ParseTokenIterator* getParseTokenIteratorParser(struct Parser *p);
struct InputChainIterator* getInputChainIteratorParser(struct Parser *p);

const char* addInputParser(struct Parser *p, const char *input);

struct ParseResult* parseInputParser(struct Parser *parser);

int hasUnparsedInputParser(struct Parser *parser);

int isCompleteParseResult(const struct ParseResult *r);
int getLengthParseResult(const struct ParseResult *r);

struct ParseTokenIterator* getParseTokenIteratorParseResult(struct ParseResult *r);

#endif
