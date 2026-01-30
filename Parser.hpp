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

using RuleTag = long long;
using ParserState = long long;

constexpr ParserState ParserState_none = 0; 

struct Action {
    ParserActType_ type = ParserActType_error;
    ParserState value = ParserState_none;
};

struct GrammarRule {
    TokenID lhsId;     
    size_t rhsSize;   
    RuleTag tag;
};

using ActionTable = std::unordered_map<TokenID, std::unordered_map<TokenID, Action>>;
using GotoTable = std::unordered_map<TokenID, std::unordered_map<TokenID, TokenID>>;
using ParserStateStack = std::vector<ParserState>;
using GrammarRuleList = std::vector<GrammarRule>;

using ReduceList = std::vector<Token>;

class ParserValueStack {
public:
    virtual int pushTerm(const Token& token) = 0;
    virtual bool pushReduced(const GrammarRule& rule) = 0;
    virtual bool pop() = 0;
private:
};

struct ParserInputArgs {
    Lexer& lexer;
    LexerSource& source;
    LexerResultInfo& lexerResInfo;
    ParserValueStack& valueStack;
    const ParserState& startState;
};

class Parser {
public:
    Parser() = default;
    Parser(Parser&& parser) {
        operator=(std::move(parser));
    }

    Parser& operator=(Parser&& parser);

    int parseNext(const ParserInputArgs& args);
    void init(ActionTable&& actionTable, GotoTable&& gotoTable, GrammarRuleList&& rules);
private:
    ActionTable mActionTable;
    GotoTable mGotoTable;
    ParserStateStack mStateStack;
    GrammarRuleList mGrammarRules;
};

#endif