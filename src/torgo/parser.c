/**
 * parser.c
 * Shell command syntax parser.
 *
 * The syntax is simple. It consists of symbols, seperated by blanks, and terminated by an operator.
 *
 *  char = 0x00 | 0x01 | ... | 0x0fe | 0xff
 *  blank = " " | "\t"
 *  endCmd = ";"
 *  opToken = cmdCmd
 *  operator = "\n" | opToken
 *  metachar = blank | opToken
 *  quotWord = '"' {char} '"'
 *  unquoWord = {char - metachar}
 *  word = { unquotWord | quotWord }
 *  token = word | operator
 *  command = word { blank word } operator
 *
 *  The parser proceeds by reading tokens from the input until an 'operator' is reached.
 *  The collection of tokens up to that point forms a parsed command, which is returned 
 *  to the caller.
 *
 *  The parse result is one of:
 *  ParseResult = Command [Token] | Incomplete | SyntaxError "error"
 *
 *  The idea of the Incomplete parse result is that when reading from an input we may get
 *  starved for data, this isn't a syntax error, the parser can still continue, it just
 *  needs more data.
 *
 *  To model the parser is modeled as an FSM.
 *
 */

#include <libc.h>

#include <torgo/charbuf.h>
#include <torgo/string.h>

/*
 * ParseToken :: (String, ParseToken)
 */
struct ParseToken {
  struct String *token;
  struct ParseToken *next;
};

void freeParseToken(struct ParseToken *tok) {
  freeString(tok->token);
  free(tok);
}

/*
 * ParseTokenIterator :: ...
 */
struct ParseTokenIterator {
  struct ParseToken *token;
};

struct ParseTokenIterator* mkParseTokenIterator(struct ParseToken * const token) {
  struct ParseTokenIterator *it = malloc(sizeof *it);
  if (! it) return NULL;

  it->token = token;
  return it;
}

void freeParseTokenIterator(struct ParseTokenIterator *it) {
  free(it);
}

/*
 * getNextParseTokenIterator :: ParseTokenIterator -> String
 *
 * Pull token Strings out of the iterator. Returl NULL when exhausted.
 */
const struct String* getNextParseTokenIterator(struct ParseTokenIterator *it) {
  if (!it || !it->token) return NULL;

  const struct String *token = it->token->token;
  it->token = it->token->next;

  return token;
}

typedef enum {
  ParsedCommand,
  ParseIncomplete,
  ParseError
} ParseResultType;

/*
 * ParseResult :: ParsedCommand [ParseToken] | ParseError String | ParseIncomplete
 */
struct ParseResult {
  ParseResultType isA;
  union {
    const struct String *error;
    struct {
      int length;
      struct ParseToken *tokens;
    } complete;
  } data;
};

/*
 * mkParseIncomplete :: ParseResult(ParseIncomplete)
 *
 * Smart constructor for a ParseIncomplete.
 */
const struct ParseResult* mkParseIncomplete(void) {
  struct ParseResult *pi = malloc(sizeof *pi);
  pi->isA = ParseIncomplete;
  pi->data.complete.length = 0;
  pi->data.complete.tokens = NULL;
  return pi;
}

/*
 * mkParseError :: String -> ParseResult(ParseError)
 *
 * Smart constructor for a ParseError
 */
const struct ParseResult* mkParseError(const struct String *err) {
  struct ParseResult *pe = malloc(sizeof *pe);
  pe->isA = ParseError;
  pe->data.error = copyString(err);
  return pe;
}

/*
 * mkParsedCommand :: [ParseToken] -> ParseResult(ParsedCommand)
 *
 * Smart constructor for a ParsedCommand.
 * The ParseToken list is expected to be in reverse order of the parse
 * so that when traversed the ParsedCommand stores the tokens in the
 * correct order.
 */
const struct ParseResult* mkParsedCommand(const struct ParseToken *tokens) {
  struct ParseResult *pc = malloc(sizeof *pc);
  pc->isA = ParsedCommand;
  pc->data.complete.length = 0;
  pc->data.complete.tokens = NULL;

  const struct ParseToken *tok0 = tokens;
  while (tok0) {
    struct ParseToken *tok = malloc(sizeof *tok);
    tok->token = copyString(tok0->token);
    tok->next = pc->data.complete.tokens;
    pc->data.complete.tokens = tok;
    pc->data.complete.length++;
    tok0 = tok0->next;
  }

  return pc;
}

/*
 * freeParseError :: ParseResult(ParseError) -> ()
 *
 * Release internal memeory of a ParseError
 */
void freeParseError(struct ParseResult *pe) {
  freeString((struct String*)pe->data.error);
}

/*
 * freeParsedCommand :: ParseResult(ParsedCommand) -> ()
 *
 * Release internal memory of a ParsesCommand
 */
void freeParsedCommand(struct ParseResult *pc) {
  while (pc->data.complete.tokens) {
    struct ParseToken *tok = pc->data.complete.tokens->next;
    freeParseToken(pc->data.complete.tokens);
    pc->data.complete.tokens = tok;
  }
  pc->data.complete.length = 0;
}

/*
 * freeParseResult :: ParseResult -> ()
 *
 * Release memory associated with any ParseResult
 */
void freeParseResult(struct ParseResult *pr) {
  switch (pr->isA) {
    case ParseError:
      freeParseError(pr);
      break;
    case ParsedCommand:
      freeParsedCommand(pr);
      break;
    case ParseIncomplete:
      break;
  }

  free(pr);
}

/*
 * getLengthParseResult :: ParseResult -> Int
 *
 * Return the length of a ParseResult, or 0
 * if not applicable.
 */
int getLengthParseResult(const struct ParseResult *pr) {
  if (pr->isA != ParsedCommand) return 0;
  return pr->data.complete.length;
}

struct InputChain {
  struct String *input;
  struct InputChain *next;
};

void freeInputChain(struct InputChain *c) {
  freeString(c->input);
  free(c);
}

struct InputChainIterator {
  const struct InputChain *chain;
};

struct InputChainIterator* mkInputChainIterator(const struct InputChain *chain) {
  struct InputChainIterator *it = malloc(sizeof *it);
  if (!it) return NULL;

  it->chain = chain;
  return it;
}

void freeInputChainIterator(struct InputChainIterator *it) {
  free(it);
}

const struct String* getNextInputChainIterator(struct InputChainIterator *it) {
  if (!it || !it->chain) return NULL;

  const struct String *input = it->chain->input;
  it->chain = it->chain->next;

  return input;
}

/*
 * Parser state is kept external to the parse.
 * This allows the parse to be resumed with additional input
 */
typedef enum {
  ParserStateReady,
  ParserStateWord,
  ParserStateQuotation,
  ParserStateEscapeInQuot,
  ParserStateEscapeInWord,
  ParserStateOperator,
  ParserStateComplete
} ParserState;

/*
 * Parser :: ...
 */
struct Parser {
  struct ParseToken *tokens;
  struct CharBuf *token;
  struct InputChain *inputChain;
  struct InputChain *last;
  const char *input;
  ParserState state;
};

/*
 * mkParser :: Parser
 *
 * Allocate a parser.
 */
struct Parser* mkParser(void) {
  struct Parser *p = malloc(sizeof *p);
  if (! p) goto exit;

  p->token = mkCharBuf(32);
  if (! p->token) goto fail;

  p->inputChain = NULL;
  p->last = NULL;
  p->input = NULL;
  p->tokens = NULL;
  p->state = ParserStateReady;

exit:
  return p;

fail:
  free(p);
  goto exit;
}

/*
 * flushTokensParser :: Parser -> ()
 *
 * Release all parsed tokens in the parser
 */
void flushTokensParser(struct Parser *p) {
  while (p->tokens) {
    struct ParseToken *tok = p->tokens->next;
    freeParseToken(p->tokens);
    p->tokens = tok;
  }

  p->tokens = NULL;
}

/*
 * flushInputChainParser :: Parser
 *
 * Release the input chain buffers
 */
void flushInputChainParser(struct Parser *p) {
  while (p->inputChain) {
    struct InputChain *c = p->inputChain->next;
    freeInputChain(p->inputChain);
    p->inputChain = c;
  }
  p->inputChain = NULL;
  p->last = NULL;
  p->input = NULL;
}

void freeParser(struct Parser *p) {
  flushTokensParser(p);
  flushInputChainParser(p);
  freeCharBuf(p->token);
  free(p);
}

/*
 * addInputParser :: Parser -> Cstr -> ()
 *
 * Add additional input to the parser input chain.
 */
const char* addInputParser(struct Parser *p, const char *input) {
  struct String *inputStr = mkString(input);
  if (! inputStr) return NULL;

  struct InputChain *link = malloc(sizeof *link);
  if (! link) goto fail;

  link->input = inputStr;
  link->next = NULL;

  if (p->last) {
    p->last->next = link;
  } else {
    p->inputChain = link;
    p->input = fromString(link->input);
  }

  p->last = link;

exit:
  return fromString(inputStr);

fail:
  freeString(inputStr);
  goto exit;
}

struct ParseTokenIterator* getParseTokenIteratorParser(struct Parser *p) {
  return mkParseTokenIterator(p->tokens);
}

struct ParseTokenIterator* getParseTokenIteratorParseResult(struct ParseResult *r) {
  if (r->isA != ParsedCommand) return NULL;
  return mkParseTokenIterator(r->data.complete.tokens);
}

struct InputChainIterator* getInputChainIteratorParser(struct Parser *p) {
  return mkInputChainIterator(p->inputChain);
}

/*
 * advanceInputParser :: Parser
 *
 * Advances the parser down the input chain.
 * Loads input from chain buffers when the present
 * buffer is exhausted.
 */
static void advanceInputParser(struct Parser *parser) {
  if (*parser->input) {
    parser->input++;
    if (! *parser->input) advanceInputParser(parser);
  } else if (parser->inputChain->next) {
    struct InputChain *c = parser->inputChain;
    parser->inputChain = parser->inputChain->next;
    parser->input = fromString(parser->inputChain->input);
    freeInputChain(c);
  } else {
    parser->last = NULL;
  }

  return;
}

/*
 * parseInputParser :: Parser -> ParseResult
 *
 * Main parse routine. Consumes as much input as possible.
 * During a parse a token is built up in a buffer, when an
 * operator is reached the token is pushed onto the parsed 
 * token list. If there isnt more input and a parse isn't complete
 * the parser returns and the caller must load more input
 * to proceed.
 */
const struct ParseResult* parseInputParser(struct Parser *parser) {
  while (*parser->input && parser->state != ParserStateComplete) {
    switch (parser->state) {
      case ParserStateComplete: break;
      case ParserStateReady:
        if (! isEmptyCharBuf(parser->token)) {
          struct ParseToken *tok = malloc(sizeof *tok);
          tok->token = mkString(fromCharBuf(parser->token));
          tok->next = parser->tokens;
          parser->tokens = tok;
          clearCharBuf(parser->token);
        }

        switch (*parser->input) {
          case ' ':
          case '\t':
            advanceInputParser(parser);
            break;
          case '\r':
          case '\n':
          case ';':
            parser->state = ParserStateOperator;
            break;
          default:
            parser->state = ParserStateWord;
        }
        break;
      case ParserStateWord:
        switch (*parser->input) {
          case '"':
            advanceInputParser(parser);
            parser->state = ParserStateQuotation;
            break;
          case '\\':
            advanceInputParser(parser);
            parser->state = ParserStateEscapeInWord;
            break;
          case ' ':
          case '\t':
          case ';':
          case '\r':
          case '\n':
            parser->state = ParserStateReady;
            break;
          default:
            appendCharBuf(parser->token, *parser->input);
            advanceInputParser(parser);
            break;
        }
        break;
      case ParserStateQuotation:
        switch (*parser->input) {
          case '\\':
            advanceInputParser(parser);
            parser->state = ParserStateEscapeInQuot;
            break;
          case '"':
            advanceInputParser(parser);
            parser->state = ParserStateWord;
            break;
          default:
            appendCharBuf(parser->token, *parser->input);
            advanceInputParser(parser);
            break;
        }
        break;
      case ParserStateEscapeInWord:
      case ParserStateEscapeInQuot:
        switch (*parser->input) {
          case 'a':
            appendCharBuf(parser->token, '\a');
            break;
          case 'b':
            appendCharBuf(parser->token, '\b');
            break;
          case 'f':
            appendCharBuf(parser->token, '\f');
            break;
          case 'n':
            appendCharBuf(parser->token, '\n');
            break;
          case 'r':
            appendCharBuf(parser->token, '\r');
            break;
          case 't':
            appendCharBuf(parser->token, '\t');
            break;
          case 'v':
            appendCharBuf(parser->token, '\v');
            break;
          default:
            appendCharBuf(parser->token, *parser->input);
            break;
        }

        advanceInputParser(parser);
        parser->state = (parser->state == ParserStateEscapeInWord ? ParserStateWord : ParserStateQuotation);
        break;
      case ParserStateOperator:
        advanceInputParser(parser);
        parser->state = ParserStateComplete;
        break;
    }
  }

  if (parser->state != ParserStateComplete) {
    return mkParseIncomplete();
  } else {
    const struct ParseResult* result = mkParsedCommand(parser->tokens);
    flushTokensParser(parser);
    clearCharBuf(parser->token);
    parser->state = ParserStateReady;
    return result;
  }
}

/*
 * hasUnparserInputParser :: Parser -> Bool
 *
 * Return True if there is still input in the chain
 */
int hasUnparsedInputParser(struct Parser *parser) {
  return ((parser->input && *parser->input != '\0') || parser->last != NULL);
}

/*
 * isCompleteParseResult :: ParserResult -> Bool
 *
 * Return True if the given ParseResult state is competed.
 */
int isCompleteParseResult(const struct ParseResult *r) {
  return r->isA == ParsedCommand;
}
