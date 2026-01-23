#include <fstream>
#include <iostream>
#include <vector>
#include <string_view>
#include "Lexer.hpp"

class StringSource : public LexerSource {
public:
	StringSource(const char* val) : mStr(val) {
		
	}

	StringSource(std::string&& val) : mStr(val) { 

	}

	int peekChar(char& ch) override;
	int nextChar(char& ch) override;
	size_t tell() const override;
	bool seek(size_t pos) override;
private:
	size_t mPos = 0;
	std::string mStr;
};

int StringSource::peekChar(char& ch) {
	if(mPos >= mStr.size()) {
		return TKN_FINISH;
	}

	ch = mStr[mPos];
	return TKN_OK;
}

int StringSource::nextChar(char& ch) {
	if(mPos >= mStr.size()) {
		return TKN_FINISH;
	}
	
	ch = mStr[mPos];
	++mPos;

	return TKN_OK;
}

size_t StringSource::tell() const {
	return mPos;
}

bool StringSource::seek(size_t pos) {
	if (pos > mStr.size()) {
		pos = mStr.size();
	}
	mPos = pos;
	return true;
}

std::string opChars = "+-*/:;,!@#%^&()[]{}.~'";

enum TokenStates {
	token_none,
	token_plus = 1,
	token_minus,
	token_mul,
	token_div,
	token_semicolon,
	token_dot_comma,
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
	token_symbol,
	token_integer,
	token_real,
	token_apostrophe,
	token_lexer_end
};

int opIds[] {
	token_plus,
	token_minus,
	token_mul,
	token_div,
	token_semicolon,
	token_dot_comma,
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

bool isOperator(char ch) {
	return opChars.find(ch) != std::string::npos;
}

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
		args.setState(token_symbol);
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
		args.setResultInfo("symbol", true);
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

void initStaticTokens(Lexer& lexer) {
	lexer.addStatic("+", {
		.id = token_plus
	});

	lexer.addStatic("-", {
		.id = token_minus
	});

	lexer.addStatic("*", {
		.id = token_mul
	});

	lexer.addStatic("/", {
		.id = token_div
	});
}

int main() {
	StringSource src("1343+0.434+gffg+4");
	Lexer lexer;

	lexer.addSwitch(token_none, start_switch);
	lexer.addSwitch(token_symbol, symbol_switch);
	lexer.addSwitch(token_integer, integer_switch);
	lexer.addSwitch(token_real, real_switch);

	lexer.addSwitch(token_lexer_end, finish_switch);
	
	initStaticTokens(lexer);

	lexer.addDynamic("int", {
		.id = token_integer
	});

	lexer.addDynamic("real", {
		.id = token_real
	});

	lexer.addDynamic("symbol", {
		.id = token_symbol
	});
	
	LexerResultInfo resultInfo;
	Token token;
	while(lexer.next(token, src, resultInfo) == TKN_OK) {
		std::cout << token.info()->id << ' ' << token.value() << std::endl;
	}

}