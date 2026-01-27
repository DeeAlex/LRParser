#ifndef PARSER_HPP
#define PARSER_HPP

#include "Lexer.hpp"
#include <list>
#include <vector>

enum ParserActType_ { 
    ParserActType_shift,
    ParserActType_reduce,
    ParserActType_accept,
    ParserActType_error
};

enum ParseStatus_ {
    ParseStatus_ok,
    ParseStatus_skip = -3,
    ParseStatus_finish = -2,
    ParseStatus_err = -1
};

using ParserState = long long;

constexpr ParserState ParserState_none = 0; 

struct Action {
    ParserActType_ type = ParserActType_error;
    ParserState value = ParserState_none;
};

struct GrammarRule {
    long long lhsId;     
    size_t rhsSize;   
};

using ActionTable = std::unordered_map<int, std::unordered_map<int, Action>>;
using GotoTable = std::unordered_map<int, std::unordered_map<int, int>>;
using ParserStateStack = std::vector<ParserState>;
using GrammarRuleList = std::vector<GrammarRule>;

struct ParserInputArgs {
    Lexer& lexer;
    LexerSource& source;
    LexerResultInfo& lexerResInfo;
    const ParserState& startState;
};

class Parser {
public:
    int parseNext(const ParserInputArgs& args);
    void init();
private:
    ActionTable mActionTable;
    GotoTable mGotoTable;
    ParserStateStack mStateStack;
    GrammarRuleList mGrammarRules;
};

#endif