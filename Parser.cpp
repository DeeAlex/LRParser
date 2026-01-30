#include "Parser.hpp"
#include "Lexer.hpp"

Parser& Parser::operator=(Parser&& parser) {
    mActionTable = std::move(parser.mActionTable);
    mGotoTable = std::move(parser.mGotoTable);
    mGrammarRules = std::move(parser.mGrammarRules);
    mStateStack = std::move(parser.mStateStack);
    return *this;
}

int Parser::parseNext(const ParserInputArgs& args) {
    Lexer& lexer = args.lexer; 
    Token tok;
    int status = lexer.peek({
        tok, args.source, args.lexerResInfo
    });

    TokenID tokenId = token_lexer_end;

    if (status == TKN_OK) {
        tokenId = tok.info()->id; 
    }

    if (status == TKN_ERR) {
        return ParseStatus_err;
    }

    if (mStateStack.empty()) {
        mStateStack.push_back(args.startState); 
    }
    
    ParserState currentState = mStateStack.back();

    auto itActionRow = mActionTable.find(currentState);
    if (itActionRow == mActionTable.end()) {
        return ParseStatus_err;
    }

    auto itAction = itActionRow->second.find(tokenId);
    if (itAction == itActionRow->second.end()) {
        return ParseStatus_err;
    }

    Action action = itAction->second;

    switch (action.type) {
        case ParserActType_shift: {
            mStateStack.push_back(action.value);
            
            lexer.next({tok, args.source, args.lexerResInfo});
            args.valueStack.pushTerm(tok);
            return ParseStatus_ok;
        }

        case ParserActType_reduce: {
            if (action.value < 0 || action.value >= (long long)mGrammarRules.size()) {
                return ParseStatus_err;
            }

            const GrammarRule& rule = mGrammarRules[action.value]; 

            for (size_t i = 0; i < rule.rhsSize; ++i) {
                if (!mStateStack.empty()) {                    
                    mStateStack.pop_back();
                }
            }

            args.valueStack.pushReduced(rule);

            ParserState topState = mStateStack.back();
            auto itGotoRow = mGotoTable.find(topState);
            if (itGotoRow == mGotoTable.end() || itGotoRow->second.find(rule.lhsId) == itGotoRow->second.end()) {
                return ParseStatus_err; 
            }

            mStateStack.push_back(mGotoTable[topState][rule.lhsId]);
            return ParseStatus_ok; 
        }

        case ParserActType_accept:
            return ParseStatus_finish;

        case ParserActType_error:
        default:
            return ParseStatus_err;
    }
}

void Parser::init(ActionTable&& actionTable, GotoTable&& gotoTable, GrammarRuleList&& rules) {
    mActionTable = std::move(actionTable);
    mGotoTable = std::move(gotoTable);
    mGrammarRules = std::move(rules);
}