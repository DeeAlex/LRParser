#include "Parser.hpp"

int Parser::parseNext(const ParserInputArgs& args) {
    Lexer& lexer = args.lexer; 
    Token tok;
    lexer.peek({
        tok, args.source, args.lexerResInfo
    });
    TokenID tokenId = tok.info()->id; 

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
            return ParseStatus_ok;
        }

        case ParserActType_reduce: {
            if (action.value < 0 || action.value >= (long long)mGrammarRules.size()) {
                return ParseStatus_err;
            }

            const auto& rule = mGrammarRules[action.value]; 

            for (size_t i = 0; i < rule.rhsSize; ++i) {
                if (!mStateStack.empty()) {
                    mStateStack.pop_back();
                }
            }

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

void Parser::init() {
    // --- STATE 0 ---
    // If we see 'id', shift to State 3
    mActionTable[0][1] = { ParserActType_shift, 3 };
    // If we see 'E', go to State 1
    mGotoTable[0][100] = 1;
    // If we see 'T', go to State 2
    mGotoTable[0][101] = 2;

    // --- STATE 1 ---
    // If we see '+', shift to State 4 (State 4 would be E -> E + . T)
    mActionTable[1][2] = { ParserActType_shift, 4 };
    // If we see EOF, Accept!
    mActionTable[1][0] = { ParserActType_accept, 0 };

    // --- STATE 2 (Reduction) ---
    // Rule: E -> T is Rule ID 1 in your code
    // In SLR, we reduce if the lookahead is in Follow(E), e.g., '+', or EOF
    mActionTable[2][2] = { ParserActType_reduce, 1 }; 
    mActionTable[2][0] = { ParserActType_reduce, 1 };

    // --- STATE 3 (Reduction) ---
    // Rule: T -> id is Rule ID 2 (or 1 depending on your index)
    mActionTable[3][2] = { ParserActType_reduce, 2 };
    mActionTable[3][0] = { ParserActType_reduce, 2 };
}