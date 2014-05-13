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

#include <manos.h>

#include <torgo/charbuf.h>
#include <torgo/dstring.h>

/*
 * ParseToken :: (String, ParseToken)
 */
typedef struct ParseToken {
  String*            token;
  struct ParseToken* next;
} ParseToken;

void freeParseToken(ParseToken *tok) {
  freeString(tok->token);
  tok->next = NULL;
  kfree(tok);
}

/*
 * ParseTokenIterator :: ...
 */
typedef struct ParseTokenIterator {
  ParseToken *token;
} ParseTokenIterator;

ParseTokenIterator* mkParseTokenIterator(ParseToken * const token) {
  ParseTokenIterator *it = kmalloc(sizeof *it);
  if (! it) return NULL;

  it->token = token;
  return it;
}

void freeParseTokenIterator(ParseTokenIterator *it) {
  kfree(it);
}

/*
 * getNextParseTokenIterator :: ParseTokenIterator -> String
 *
 * Pull token Strings out of the iterator. Returl NULL when exhausted.
 */
const String* getNextParseTokenIterator(ParseTokenIterator *it) {
  if (!it || !it->token) return NULL;

  const String *token = it->token->token;
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
typedef struct ParseResult {
  ParseResultType isA;
  union {
    const String *error;
    struct {
      int length;
      ParseToken *tokens;
    } complete;
  } data;
} ParseResult;

/*
 * mkParseIncomplete :: ParseResult(ParseIncomplete)
 *
 * Smart constructor for a ParseIncomplete.
 */
const ParseResult* mkParseIncomplete(void) {
  ParseResult *pi = kmalloc(sizeof *pi);
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
const ParseResult* mkParseError(const String *err) {
  ParseResult *pe = kmalloc(sizeof *pe);
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
const ParseResult* mkParsedCommand(const ParseToken *tokens) {
  ParseResult *pc = kmalloc(sizeof *pc);
  pc->isA = ParsedCommand;
  pc->data.complete.length = 0;
  pc->data.complete.tokens = NULL;

  const ParseToken *tok0 = tokens;
  while (tok0) {
    ParseToken *tok = kmalloc(sizeof *tok);
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
void freeParseError(ParseResult *pe) {
  freeString((String*)pe->data.error);
}

/*
 * freeParsedCommand :: ParseResult(ParsedCommand) -> ()
 *
 * Release internal memory of a ParsesCommand
 */
void freeParsedCommand(ParseResult *pc) {
  while (pc->data.complete.tokens) {
    ParseToken *tok = pc->data.complete.tokens->next;
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
void freeParseResult(ParseResult *pr) {
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

  kfree(pr);
}

/*
 * getLengthParseResult :: ParseResult -> Int
 *
 * Return the length of a ParseResult, or 0
 * if not applicable.
 */
int getLengthParseResult(const ParseResult *pr) {
  if (pr->isA != ParsedCommand) return 0;
  return pr->data.complete.length;
}

typedef struct InputChain {
  String*            input;
  struct InputChain* next;
} InputChain;

void freeInputChain(InputChain *c) {
  freeString(c->input);
  kfree(c);
}

typedef struct InputChainIterator {
  const InputChain *chain;
} InputChainIterator;

InputChainIterator* mkInputChainIterator(const InputChain *chain) {
  InputChainIterator *it = kmalloc(sizeof *it);
  if (!it) return NULL;

  it->chain = chain;
  return it;
}

void freeInputChainIterator(InputChainIterator *it) {
  kfree(it);
}

const String* getNextInputChainIterator(InputChainIterator *it) {
  if (!it || !it->chain) return NULL;

  const String *input = it->chain->input;
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
typedef struct Parser {
  ParseToken* tokens;
  CharBuf*    token;
  InputChain* inputChain;
  InputChain* last;
  const char* input;
  ParserState state;
} Parser;

/*
 * mkParser :: Parser
 *
 * Allocate a parser.
 */
Parser* mkParser(void) {
  struct Parser *p = kmalloc(sizeof *p);
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
  kfree(p);
  goto exit;
}

/*
 * flushTokensParser :: Parser -> ()
 *
 * Release all parsed tokens in the parser
 */
void flushTokensParser(Parser *p) {
  while (p->tokens) {
    ParseToken *tok = p->tokens->next;
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
void flushInputChainParser(Parser *p) {
  while (p->inputChain) {
    InputChain *c = p->inputChain->next;
    freeInputChain(p->inputChain);
    p->inputChain = c;
  }
  p->inputChain = NULL;
  p->last = NULL;
  p->input = NULL;
}

void freeParser(Parser *p) {
  flushTokensParser(p);
  flushInputChainParser(p);
  freeCharBuf(p->token);
  kfree(p);
}

/*
 * addInputParser :: Parser -> Cstr -> ()
 *
 * Add additional input to the parser input chain.
 */
const char* addInputParser(Parser *p, const char *input) {
  String *inputStr = mkString(input);
  if (! inputStr) return NULL;

  InputChain *link = kmalloc(sizeof *link);
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

ParseTokenIterator* getParseTokenIteratorParser(Parser *p) {
  return mkParseTokenIterator(p->tokens);
}

ParseTokenIterator* getParseTokenIteratorParseResult(ParseResult *r) {
  if (r->isA != ParsedCommand) return NULL;
  return mkParseTokenIterator(r->data.complete.tokens);
}

InputChainIterator* getInputChainIteratorParser(Parser *p) {
  return mkInputChainIterator(p->inputChain);
}

/*
 * advanceInputParser :: Parser
 *
 * Advances the parser down the input chain.
 * Loads input from chain buffers when the present
 * buffer is exhausted.
 */
static void advanceInputParser(Parser *parser) {
  if (*parser->input) {
    parser->input++;
    if (! *parser->input) advanceInputParser(parser);
  } else if (parser->inputChain->next) {
    InputChain *c = parser->inputChain;
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
const ParseResult* parseInputParser(Parser *parser) {
  while (*parser->input && parser->state != ParserStateComplete) {
    switch (parser->state) {
      case ParserStateComplete: break;
      case ParserStateReady:
        if (! isEmptyCharBuf(parser->token)) {
          ParseToken *tok = kmalloc(sizeof *tok);
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
    const ParseResult* result = mkParsedCommand(parser->tokens);
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
int hasUnparsedInputParser(Parser *parser) {
  return ((parser->input && *parser->input != '\0') || parser->last != NULL);
}

/*
 * isCompleteParseResult :: ParserResult -> Bool
 *
 * Return True if the given ParseResult state is competed.
 */
int isCompleteParseResult(const ParseResult *r) {
  return r->isA == ParsedCommand;
}
