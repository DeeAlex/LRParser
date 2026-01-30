#include "ParserBuilder.hpp"
#include "Lexer.hpp"
#include "LexerDefs.hpp"
#include "LexerSources.hpp"

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

void ParserBuilder::computeAllFirstSets(const std::vector<TokRule>& rules) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& rule : rules) {
            TokenID lhs = rule.lhs.info()->id;
            bool derivesEpsilon = true;

            for (const auto& sym : rule.rhs) {
                if (sym.info()->category == TokenCategory_term) {
                    if (mFirstSets[lhs].insert(sym.info()->id).second) {
						changed = true;
					}
                    derivesEpsilon = false;
                    break;
                } else {
                    bool symHasEpsilon = false;
                    for (TokenID termId : mFirstSets[sym.info()->id]) {
                        if (termId == token_none) {
                            symHasEpsilon = true;
                        } else {
                            if (mFirstSets[lhs].insert(termId).second) {
								changed = true;
							}
                        }
                    }
                    
                    if (!symHasEpsilon) {
                        derivesEpsilon = false;
                        break;
                    }
                }
            }

            if (derivesEpsilon) {
                if (mFirstSets[lhs].insert(token_none).second) {
					changed = true;
				}
            }
        }
    }
}

std::set<TokenID> ParserBuilder::computeFirst(const TokRule& rule, int startIdx, TokenID currentLookahead) {
    std::set<TokenID> result;
    bool allDeriveEpsilon = true;

    for (size_t i = startIdx; i < rule.rhs.size(); ++i) {
        TokenID symId = rule.rhs[i].info()->id;
        
        if (rule.rhs[i].info()->category == TokenCategory_term) {
            result.insert(symId);
            allDeriveEpsilon = false;
            break; 
        }

        bool hasEpsilon = false;
        for (TokenID f : mFirstSets[symId]) {
            if (f == token_none) {
                hasEpsilon = true;
            } else {
                result.insert(f);
            }
        }

        if (!hasEpsilon) {
            allDeriveEpsilon = false;
            break;
        }
    }

    if (allDeriveEpsilon) {
        result.insert(currentLookahead);
    }
    return result;
}

void ParserBuilder::computeClosure(StateSet& set, const std::vector<TokRule>& rules) {
    bool changed = true;
    while (changed) {
        changed = false;
        StateSet newItems;
        for (const LRItem& item : set) {
            const TokRule& rule = rules[item.ruleIndex];
            if (item.dotPos < rule.rhs.size()) {
                Token nextSymbol = rule.rhs[item.dotPos];
                if (nextSymbol.info()->category == TokenCategory_nonterm) {
                    std::set<TokenID> lookaheads = computeFirst(rule, item.dotPos + 1, item.lookaheadId);
                    
                    for (int rIdx = 0; rIdx < rules.size(); ++rIdx) {
                        if (rules[rIdx].lhs.info()->id == nextSymbol.info()->id) {
                            for (int la : lookaheads) {
                                if (set.find(LRItem {rIdx, 0, la }) == set.end()) {
                                    newItems.insert(LRItem {rIdx, 0, la });
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        set.insert(newItems.begin(), newItems.end());
    }
}

StateSet ParserBuilder::computeGoto(const StateSet& items, TokenID symbolId, const std::vector<TokRule>& rules) {
    StateSet movedItems;
    for (const auto& item : items) {
        const auto& rule = rules[item.ruleIndex];
        if (item.dotPos < rule.rhs.size() && rule.rhs[item.dotPos].info()->id == symbolId) {
            movedItems.insert({item.ruleIndex, item.dotPos + 1, item.lookaheadId});
        }
    }
    
    computeClosure(movedItems, rules);
    return movedItems;
}

void ParserBuilder::buildTables(Parser& parser, const std::vector<TokRule>& rules) {
    std::vector<StateSet> states;
    std::map<StateSet, int> stateToIndex;
    std::queue<int> worklist;

    StateSet startSet;
    startSet.insert({0, 0, token_lexer_end}); 
    computeClosure(startSet, rules);
    
    states.push_back(startSet);
    stateToIndex[startSet] = 0;
    worklist.push(0);

	ActionTable actionTable;
	GotoTable gotoTable;

    while (!worklist.empty()) {
        int currIdx = worklist.front();
        worklist.pop();
        const StateSet currSet = states[currIdx];

        std::set<TokenID> processedSymbols;

        for (const LRItem& item : currSet) {
            const TokRule& rule = rules[item.ruleIndex];
            
			if (item.dotPos == rule.rhs.size()) {
                if (item.ruleIndex == 0 && item.lookaheadId == token_lexer_end) {
                    actionTable[currIdx][token_lexer_end] = {ParserActType_accept, 0};
                } else {
                    actionTable[currIdx][item.lookaheadId] = {ParserActType_reduce, (ParserState)item.ruleIndex};
                }
                continue;
            }

            TokenID symId = rule.rhs[item.dotPos].info()->id;
            if (processedSymbols.count(symId)) {
                continue;
            }
            processedSymbols.insert(symId);

            StateSet nextSet = computeGoto(currSet, symId, rules);
            if (nextSet.empty()) {
                continue;
            }

            if (stateToIndex.find(nextSet) == stateToIndex.end()) {
                stateToIndex[nextSet] = states.size();
                states.push_back(nextSet);
                worklist.push(stateToIndex[nextSet]);
            }

            int nextIdx = stateToIndex[nextSet];
            if (rule.rhs[item.dotPos].info()->category == TokenCategory_nonterm) {
                gotoTable[currIdx][symId] = nextIdx;
            } else {
                actionTable[currIdx][symId] = {ParserActType_shift, (ParserState)nextIdx};
            }
        }
    }

	GrammarRuleList ruleList;
	for (auto& val : rules) {
		ruleList.push_back(GrammarRule {
			.lhsId = val.lhs.info()->id,
			.rhsSize = val.rhs.size(),
            .tag = val.tag
		});
	}

	parser.init(std::move(actionTable), std::move(gotoTable), std::move(ruleList));
}

ParserBuilder& ParserBuilder::initGrammarLexer() {
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
    gramLexerBuilder.addStatic("real", {
		.id = token_real
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
    gramLexerBuilder.addStatic("P", {
		.id = ParserStates_pow,
		.category = TokenCategory_nonterm
	});
	gramLexerBuilder.addStatic("F", {
		.id = ParserStates_fact,
		.category = TokenCategory_nonterm
	});

	gramLexerBuilder.addState(token_id, lexer_def_symbol_switch);
	gramLexerBuilder.addState(token_op, grammar_lexer_implies);
	gramLexerBuilder.addState(token_any, lexer_any_visible_switch);

	mGrammarLexer = gramLexerBuilder.withStandardOperators().build();
    return *this;
}

ParserBuilder& ParserBuilder::loadGrammar(const std::span<const StrRule>& grammar) {
	LexerResultInfo resultInfo;
	
	std::vector<TokRule> ruleArr;
	StateSet stateSet;
	for (int i = 0 ; i < std::size(grammar); ++i) {
		StringSource source(grammar[i].rule);
		
		Token lhs;
		mGrammarLexer.next({
			.token = lhs,
			.source = source,
			.debug = resultInfo,
			.initState = token_id,
		});
	
		Token imp;
		mGrammarLexer.next({
			.token = imp,
			.source = source,
			.debug = resultInfo,
			.initState = token_op,
		});
	
		Token rhs;
		std::vector<Token> rhsVec;
		while (TKN_OK == mGrammarLexer.next({
			.token = rhs,
			.source = source,
			.debug = resultInfo,
			.initState = token_any,
		})) {
			rhsVec.emplace_back(rhs);
		}		
		
		ruleArr.emplace_back(TokRule {
			.lhs = lhs,
			.rhs = std::move(rhsVec),
            .tag = grammar[i].tag
		});
	}

	stateSet.emplace(LRItem {
		.ruleIndex = 0, 
		.dotPos = 0,
		.lookaheadId = token_lexer_end
	});

	computeAllFirstSets(ruleArr);
	buildTables(mParser, ruleArr);
    return *this;
}