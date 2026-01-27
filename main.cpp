#include <cstdio>
#include <iostream>
#include <iterator>
#include <map>
#include <vector>
#include <set>
#include "Lexer.hpp"
#include "LexerBuilder.hpp"
#include "LexerDefs.hpp"
#include "LexerSources.hpp"
#include "Parser.hpp"

enum ParserStates {
	ParserStates_stmt = 1000000,
	ParserStates_expr,
	ParserStates_term,
	ParserStates_fact
};

int grammar_lexer_implies(const TokenSwitchArgs& args) {
	bool isBlank = isspace(args.ch) || iscntrl(args.ch) || isblank(args.ch);

	if (args.tokVal == "->" && isBlank) {
		args.setResultInfo("->", false);
		return TKN_FINISH;
	} else if (args.tokVal == "->") {
		args.msg = "Unexpected character next to '->'";
		return TKN_ERR;
	}

	if (isBlank) {
		return TKN_SKIP;
	}

	if(args.tokVal.empty() && args.ch == '-') {
		return TKN_OK;
	}

	if (args.tokVal == "-" && args.ch == '>') {
		return TKN_OK;
	}

	
	args.msg = "Expected '->'";
	return TKN_ERR;
}

struct RawRule {
	Token lhs;
	std::vector<Token> rhs;
};

struct LRItem {
    int ruleIndex;
    int dotPos;
    int lookaheadId;

    bool operator<(const LRItem& other) const {
        if (ruleIndex != other.ruleIndex) return ruleIndex < other.ruleIndex;
        if (dotPos != other.dotPos) return dotPos < other.dotPos;
        return lookaheadId < other.lookaheadId;
    }
};

using StateSet = std::set<LRItem>;

void applyClosure(StateSet& set, const std::vector<RawRule>& rules) {
    bool changed = true;
    while (changed) {
        changed = false;
        StateSet currentSet = set; 
        for (const auto& item : currentSet) {
            const RawRule& rule = rules[item.ruleIndex];
            
            if (item.dotPos < rule.rhs.size()) {
                const Token& nextSymbol = rule.rhs[item.dotPos];

                if (nextSymbol.info()->category == TokenCategory_nonterm) {
                    std::vector<Token> beta;
                    for (size_t i = item.dotPos + 1; i < rule.rhs.size(); ++i) {
                        beta.push_back(rule.rhs[i]);
                    }
                    
                    //TODO: std::set<int> firstSet = computeFirst(beta, item.lookaheadId, rules);

                    // for (int i = 0; i < rules.size(); ++i) {
                    //     if (rules[i].lhs.info()->id == nextSymbol.info()->id) {
                    //         for (int lookahead : firstSet) {
                    //             LRItem newItem{ .ruleIndex = i, .dotPos = 0, .lookaheadId = lookahead };
                    //             if (set.insert(newItem).second) {
                    //                 changed = true;
                    //             }
                    //         }
                    //     }
                    // }
                }
            }
        }
    }
}

StateSet computeGoto(const StateSet& current, int symbolId, const std::vector<RawRule>& rules) {
    StateSet nextState;
    for (const auto& item : current) {
        const auto& rule = rules[item.ruleIndex];
        if (item.dotPos < rule.rhs.size() && rule.rhs[item.dotPos].info()->id == symbolId) {
            nextState.insert({item.ruleIndex, item.dotPos + 1});
        }
    }
    applyClosure(nextState, rules);
    return nextState;
}

void parseGrammar() {
	const char* grammar[] = {
		"S -> E",
		"E -> T + E",
		"E -> T",
		"T -> int"		
	};

	LexerBuilder gramLexerBuilder;
	gramLexerBuilder.addStatic("->", {
		.id = token_op
	});
	gramLexerBuilder.addDynamic("id", {
		.id = token_id
	});
	gramLexerBuilder.addStatic("int", {
		.id = token_integer
	});
	gramLexerBuilder.addStatic("NONE", {
		.id = token_none
	});
	gramLexerBuilder.addStatic("S", {
		.id = ParserStates_stmt,
		.category = TokenCategory_nonterm
	});
	gramLexerBuilder.addStatic("E", {
		.id = ParserStates_expr,
		.category = TokenCategory_nonterm
	});
	gramLexerBuilder.addStatic("T", {
		.id = ParserStates_term,
		.category = TokenCategory_nonterm
	});
	gramLexerBuilder.addStatic("F", {
		.id = ParserStates_fact,
		.category = TokenCategory_nonterm
	});

	gramLexerBuilder.addState(token_id, lexer_def_symbol_switch);
	gramLexerBuilder.addState(token_op, grammar_lexer_implies);
	gramLexerBuilder.addState(token_any, lexer_any_visible_switch);

	Lexer lexer = gramLexerBuilder.build();
	LexerResultInfo resultInfo;
	
	std::vector<RawRule> ruleArr;
	StateSet stateSet;
	for (int i = 0 ; i < std::size(grammar); ++i) {
		StringSource source(grammar[i]);
		
		Token lhs;
		lexer.next({
			.token = lhs,
			.source = source,
			.debug = resultInfo,
			.initState = token_id,
		});
	
		Token imp;
		lexer.next({
			.token = imp,
			.source = source,
			.debug = resultInfo,
			.initState = token_op,
		});
	
		Token rhs;
		std::vector<Token> rhsVec;
		while (TKN_OK == lexer.next({
			.token = rhs,
			.source = source,
			.debug = resultInfo,
			.initState = token_any,
		})) {
			rhsVec.emplace_back(rhs);
		}		
		
		ruleArr.emplace_back(RawRule {
			.lhs = lhs,
			.rhs = std::move(rhsVec)
		});
	}

	stateSet.emplace(LRItem {
		.ruleIndex = 0, 
		.dotPos = 0,
	});

	applyClosure(stateSet, ruleArr);

	printf("");
}

int main() {
	parseGrammar();
	StringSource src("1343+0.434+gffg+4");
	LexerBuilder builder;
	Lexer lexer = builder.withDefaultStates().withStandardOperators().build();
	
	LexerResultInfo resultInfo;
	Token token;

	while(lexer.next({
		.token = token,
		.source = src,
		.debug = resultInfo,
	}) == TKN_OK) {
		std::cout << token.info()->id << ' ' << token.value() << std::endl;
	}
}