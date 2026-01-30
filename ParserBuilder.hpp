#ifndef PARSERBUILDER_HPP
#define PARSERBUILDER_HPP

#include <initializer_list>
#include <map>
#include <set>
#include <ranges>
#include <span>
#include <queue>
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Lexer.hpp"
#include "LexerBuilder.hpp"

int grammar_lexer_implies(const TokenSwitchArgs& args);

using FirstSet = std::map<TokenID, std::set<TokenID>>;

struct LRItem {
    int ruleIndex{};
    int dotPos{};
    TokenID lookaheadId{};

    bool operator<(const LRItem& other) const {
        if (ruleIndex != other.ruleIndex) {
            return ruleIndex < other.ruleIndex;
		}
        if (dotPos != other.dotPos) {
            return dotPos < other.dotPos;
		}
        return lookaheadId < other.lookaheadId;
    }
};

using StateSet = std::set<LRItem>;
using RuleStrList = std::initializer_list<const char*>;

struct TokRule {
    Token lhs;
    std::vector<Token> rhs;
    RuleTag tag;
};

enum ParserStates {
	ParserStates_stmt = 10000000,
	ParserStates_expr,
	ParserStates_term,
    ParserStates_pow,
	ParserStates_fact,
    ParserStates_CUSTOM
};

struct StrRule {
    const char* rule{};
    RuleTag tag{};
};

class ParserBuilder {
public:
    ParserBuilder& initGrammarLexer();
    Lexer& getGrammarLexer() {
        return mGrammarLexer;
    }
    ParserBuilder& loadGrammar(const std::span<const StrRule>& grammar);
    Parser build() {
        return std::move(mParser);
    }
private:
    std::set<TokenID> computeFirst(const TokRule& rule, int startIdx, TokenID currentLookahead);
    void computeAllFirstSets(const std::vector<TokRule>& rules);
    void computeClosure(StateSet& set, const std::vector<TokRule>& rules);
    StateSet computeGoto(const StateSet& items, TokenID symbolId, const std::vector<TokRule>& rules);
    void buildTables(Parser& parser, const std::vector<TokRule>& rules);
private:
    Parser mParser;
    Lexer mGrammarLexer;
    FirstSet mFirstSets;
};

#endif