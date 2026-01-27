#include "Lexer.hpp"

bool TokenSwitchArgs::setResultInfo(const char* name, bool isDyn) const {
	const TokenInfo* token = lexer->getTokenInfo(name, isDyn);

	if (!token) {
		return false;
	}
	resultInfo = token;
	return true;
}

int Lexer::callCheckers(const TokenSwitchArgs& args) {
	auto& list = mCheckers[args.state];

	for(auto& func : list) {
		int status = func(TokenSwitchArgs {
			this,
			args.state,
			args.tokVal,
			args.msg,
			args.resultInfo,
			args.ch,
		});

		if(status != TKN_OK) {
			return status;
		}
	}
	return mCheckers.size() > 0 ? TKN_OK : TKN_ERR;
}

int Lexer::next(const LexerInputArgs& args) {
	int status = TKN_OK;
	char currCh = 0;
	TokenID state = args.initState;
	const TokenInfo* resultInfo = nullptr;
	TokenVal result;
	LexerMsg msg;

	size_t col = 0;
	size_t line = 0;

	Token& token = args.token;
	LexerSource& source = args.source;
	LexerResultInfo& debug = args.debug;

	col = debug.col;
	line = debug.line;

	if(source.peekChar(currCh) == TKN_FINISH) {
		return TKN_FINISH;
	}

	while(status == TKN_OK) {
		status = source.peekChar(currCh);
		
		bool skip = false;
		if(status == TKN_SKIP) {
			source.nextChar(currCh);
			skip = true;
		}

		if (skip) {
			++col;		
			if (currCh == '\n') {
				col = 0;
				++line;
			}
			continue;
		}

		if(status == TKN_FINISH) {
			currCh = '\0';
			status = TKN_OK;
		}

		if(status != TKN_OK) {
			break;
		}

		int checkerStatus = callCheckers(TokenSwitchArgs {
			this,
			state,
			result,
			msg,
			resultInfo,
			currCh,
		});

		if (checkerStatus == TKN_ERR) {
			return TKN_ERR;
		}

		if(checkerStatus == TKN_SKIP) {
			source.nextChar(currCh);
			skip = true;
		}

		if (skip) {
			++col;		
			if (currCh == '\n') {
				col = 0;
				++line;
			}
			continue;
		}

		if (checkerStatus == TKN_ERR) {
			return TKN_ERR;
		}
		
		if(checkerStatus == TKN_FINISH) {
			if(!resultInfo) {
				return TKN_ERR;
			}
			
			token = Token(resultInfo, result);
			debug.col = col;
			debug.line = line;
			debug.curResult = std::move(result);
			debug.state = state;
			debug.tokenInfo = resultInfo;

			return TKN_OK;
		}

		source.nextChar(currCh);
		++col;		
		if (currCh == '\n') {
			col = 0;
			++line;
		}

		if (currCh == '\0') {
			break;
		}

		result += currCh;
	}

	return TKN_FINISH;
}

int Lexer::peek(const LexerInputArgs& args) {
	size_t srcOff = args.source.tell();
	int status = next(args);
	args.source.seek(srcOff);
	return status;	
}

void Lexer::addSwitch(TokenID state, TokenSwitch checker) {
	mCheckers[state].push_back(checker);
}

const TokenInfo* Lexer::getTokenInfo(const char* name, bool isDyn) {
	auto& map = isDyn ? mDynamicTokens : mStaticTokens;
	auto iter = map.find(name);

	if(iter != map.end()) {
		return &iter->second;
	}
	return nullptr;
}

const TokenInfo* Lexer::addStatic(const char* value, const TokenInfo& info) {
	auto& val = mStaticTokens[value];
	val = info;
	return &val;
}

const TokenInfo* Lexer::addDynamic(const char* value, const TokenInfo& info) {
	auto& val = mDynamicTokens[value];
	val = info;
	return &val;
}

const TokenInfo* Lexer::getStatic(const char* value) {
	auto iter = mStaticTokens.find(value);
	
	if(iter == mStaticTokens.end()) {
		return nullptr;
	}

	return &iter->second;
}

const TokenInfo* Lexer::getDynamic(const char* value) {
	auto iter = mDynamicTokens.find(value);
	
	if(iter == mDynamicTokens.end()) {
		return nullptr;
	}

	return &iter->second;
}
