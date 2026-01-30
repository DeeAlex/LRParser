#ifndef LEXER_DEFS
#define LEXER_DEFS

enum TokenStates {
	token_lexer_end = -2,
	token_none = 0,
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
	token_id,
	token_integer,
	token_real,
	token_quote,
	token_dquote,
	token_greater,
	token_less,
	token_dollar,
    token_vbar,
	token_or,
    token_op,
    token_any,
    token_new_line,
};

enum TokenCategory {
	TokenCategory_term,
	TokenCategory_nonterm,
	TokenCategory_act,
	TokenCategory_class
};

#endif
