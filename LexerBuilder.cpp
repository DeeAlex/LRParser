#include "LexerBuilder.hpp"
#include "Lexer.hpp"
#include "LexerDefs.hpp"
#include <cctype>

static std::string opChars = "+-*/:;,!@#%^&()[]{}.~'\"><$";

bool isOperator(char ch) {
	return opChars.find(ch) != std::string::npos;
}

int lexer_def_start_switch(const TokenSwitchArgs& args) {
	if (isspace(args.ch)) {
		return TKN_SKIP;
	}
	
	if(iscntrl(args.ch)) {
		return TKN_SKIP;
	}

	if(isdigit(args.ch)) {
		args.setState(token_integer);
		return TKN_OK;
	}

	if(isalpha(args.ch)) {
		args.setState(token_id);
		return TKN_OK;
	}

	size_t opId  = opChars.find(args.ch);
	
	if(opId == std::string::npos) {
		return TKN_ERR;
	} else {
		args.setState(token_lexer_end);
		args.setResultInfo(std::string(&args.ch, 1).c_str(), false);
		return TKN_OK;
	}
	return TKN_OK;
}

int lexer_def_symbol_switch(const TokenSwitchArgs& args) {    
	if(isalnum(args.ch)) {
		return TKN_OK;
	}

	if(isspace(args.ch) || iscntrl(args.ch) || isOperator(args.ch)) {	
        const TokenInfo* info = args.lexer->getStatic(args.tokVal.c_str());

        if (args.setResultInfo(args.tokVal.c_str(), false)) {
            args.resultInfo = info;
        } else {
            args.setResultInfo("id", true);
        }
		return TKN_FINISH;
	}
	return TKN_OK;
}

int lexer_any_visible_switch(const TokenSwitchArgs& args) {
    bool isBlank = isspace(args.ch) || iscntrl(args.ch) || isblank(args.ch);
    if(args.tokVal.size() && isBlank) {	
	    const TokenInfo* info = args.lexer->getStatic(args.tokVal.c_str());

        if (args.setResultInfo(args.tokVal.c_str(), false)) {
            args.resultInfo = info;
        } else {
            args.setResultInfo("id", true);
        }	
        return TKN_FINISH;
    }

    if (isspace(args.ch)) {
		return TKN_SKIP;
	}
	
	if(iscntrl(args.ch)) {
		return TKN_SKIP;
	}

    if(isalnum(args.ch)) {
		return TKN_OK;
	}
    return TKN_OK;
}

int lexer_def_integer_switch(const TokenSwitchArgs& args) {
	if(isdigit(args.ch)) {
		return TKN_OK;
	}

	if(args.ch == '.') {
        args.setState(token_real);
        return TKN_OK;
    }

	if(isspace(args.ch) || iscntrl(args.ch) || isOperator(args.ch)) {
		args.setResultInfo("int", true);
		return TKN_FINISH;
	}

	return TKN_ERR;
}

int lexer_def_real_switch(const TokenSwitchArgs& args) {
	if(isdigit(args.ch)) {
		return TKN_OK;
	}

	if(isspace(args.ch) || iscntrl(args.ch) || isOperator(args.ch)) {
		args.setResultInfo("real", true);
		return TKN_FINISH;
	}

	return TKN_ERR;
}

int lexer_def_finish_switch(const TokenSwitchArgs& args) {	
	return TKN_FINISH;
}

int opIds[] {
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
	token_tilda
};

int start_switch(const TokenSwitchArgs& args) {
	if (isspace(args.ch)) {
		return TKN_SKIP;
	}
	
	if(iscntrl(args.ch)) {
		return TKN_SKIP;
	}

	if(isdigit(args.ch)) {
		args.setState(token_integer);
		return TKN_OK;
	}

	if(isalpha(args.ch)) {
		args.setState(token_id);
		return TKN_OK;
	}

	size_t opId  = opChars.find(args.ch);
	
	if(opId == std::string::npos) {
		return TKN_ERR;
	} else {
		args.setState(token_lexer_end);
		args.setResultInfo(std::string(&args.ch, 1).c_str(), false);
		return TKN_OK;
	}
	return TKN_OK;
}

int symbol_switch(const TokenSwitchArgs& args) {
	if(isalnum(args.ch)) {
		return TKN_OK;
	}

	if(isspace(args.ch) || iscntrl(args.ch) || isOperator(args.ch)) {	
		args.setResultInfo("id", true);
		return TKN_FINISH;
	}
	return TKN_OK;
}

int integer_switch(const TokenSwitchArgs& args) {
	if(isdigit(args.ch)) {
		return TKN_OK;
	}

	if(args.ch == '.') {
        args.setState(token_real);
        return TKN_OK;
    }

	if(isspace(args.ch) || iscntrl(args.ch) || isOperator(args.ch)) {
		args.setResultInfo("int", true);
		return TKN_FINISH;
	}
	return TKN_ERR;
}

int real_switch(const TokenSwitchArgs& args) {
	if(isdigit(args.ch)) {
		return TKN_OK;
	}

	if(isspace(args.ch) || iscntrl(args.ch) || isOperator(args.ch)) {
		args.setResultInfo("real", true);
		return TKN_FINISH;
	}
	return TKN_ERR;
}

int finish_switch(const TokenSwitchArgs& args) {	
	return TKN_FINISH;
}