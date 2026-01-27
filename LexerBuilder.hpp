#ifndef LEXERBUILDER_HPP
#define LEXERBUILDER_HPP

#include "LexerDefs.hpp"
#include "Lexer.hpp"

const std::string LEXER_DEFAULT_OP_CHARS = "+-*/:;,!@#%^&()[]{}.~'\"><$|";
constexpr int LEXER_DEFAULT_OP_IDS[] {
	token_plus,
	token_minus,
	token_mul,
	token_div,
	token_colon,
	token_semicolon,
	token_comma,
	token_not,
	token_at,
	token_sharp,
	token_perc,
	token_circ,
	token_and,
	token_bracket_open,
	token_bracket_close,
	token_sqr_open,
	token_sqr_close,
	token_brace_open,
	token_brace_close,
	token_dot,
	token_tilda,
    token_quote,
    token_dquote,
    token_greater,
    token_less,
    token_dollar,
    token_vbar
};

int lexer_def_start_switch(const TokenSwitchArgs& args);
int lexer_def_symbol_switch(const TokenSwitchArgs& args);
int lexer_def_integer_switch(const TokenSwitchArgs& args);
int lexer_def_real_switch(const TokenSwitchArgs& args);
int lexer_def_finish_switch(const TokenSwitchArgs& args);
int lexer_any_visible_switch(const TokenSwitchArgs& args);

class LexerBuilder {
public:
    LexerBuilder() = default;

    LexerBuilder& addState(int state, TokenSwitchFunc func) {
        mLexer.addSwitch(state, func);
        return *this;
    }

    LexerBuilder& addOperator(const std::string& op, int id) {
        mLexer.addStatic(op.c_str(), { .id = (int)id });
        return *this;
    }

    LexerBuilder& addStatic(const char* value, const TokenInfo& info) {
        mLexer.addStatic(value, info);
        return *this;
    }

    LexerBuilder& addDynamic(const char* value, const TokenInfo& info) {
        mLexer.addDynamic(value, info);
        return *this;
    }

    LexerBuilder& withDefaultStates() {        
        mLexer.addSwitch(token_none, lexer_def_start_switch);
        mLexer.addSwitch(token_lexer_end, lexer_def_finish_switch);
        mLexer.addSwitch(token_id, lexer_def_symbol_switch);
        mLexer.addSwitch(token_integer, lexer_def_integer_switch);
        mLexer.addSwitch(token_real, lexer_def_real_switch);

        mLexer.addDynamic("int", {
            .id = token_integer
        });
        mLexer.addDynamic("real", {
            .id = token_real
        });
        mLexer.addDynamic("id", {
            .id = token_id
        });
        return *this;
    }

    LexerBuilder& withStandardOperators() {
        for (size_t i = 0; i < LEXER_DEFAULT_OP_CHARS.size(); ++i) {
            std::string op(1, LEXER_DEFAULT_OP_CHARS[i]);
            mLexer.addStatic(op.c_str(), { .id = LEXER_DEFAULT_OP_IDS[i] });
        }
        return *this;
    }

    Lexer build() {
        return std::move(mLexer);
    }
private:
    Lexer mLexer;
};

#endif