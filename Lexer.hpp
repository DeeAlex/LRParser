#ifndef LEXER_HPP
#define LEXER_HPP

#include "LexerDefs.hpp"
#include <functional>
#include <string>
#include <unordered_map>
#include <list>

constexpr int TKN_ERR = -1;
constexpr int TKN_FINISH = -2;
constexpr int TKN_SKIP = -3;
constexpr int TKN_OK = 0;
constexpr int TKN_NO_ID = 0;

class Lexer;

using TokenID = long long;
using TokenVal = std::string;
using LexerMsg = std::string;

struct TokenInfo {
	TokenID id = TKN_NO_ID;
	TokenID category = TKN_NO_ID;
	TokenVal value;
};

struct LexerResultInfo {
	size_t line{};
	size_t col{};
	const TokenInfo* tokenInfo{};
	TokenID state{};
	TokenVal curResult{};
	LexerMsg message{};
};

struct TokenSwitchArgs {
	Lexer* lexer{};
	TokenID& state;
	TokenVal& tokVal;
	LexerMsg& msg;
	const TokenInfo*& resultInfo;
	char ch;

	inline void setState(TokenID state) const {
		this->state = state;
	} 

	bool setResultInfo(const char* name, bool isDyn) const;
};

using TokenSwitchFunc = int (*)(const TokenSwitchArgs& args);
using TokenSwitch = std::function<int (const TokenSwitchArgs& args)>;
using TokenCheckerMap = std::unordered_map<TokenID, std::list<TokenSwitch>>;

class LexerSource {
public:
	virtual ~LexerSource() = default;
	virtual int peekChar(char& ch) = 0;
	virtual int nextChar(char& ch) = 0;
	virtual size_t tell() const = 0;
	virtual bool seek(size_t pos) = 0;
};

using TokenMap = std::unordered_map<TokenVal, TokenInfo>;

struct Token {
public:
	Token(const TokenInfo* info = nullptr, const TokenVal& val = "") : mInfo(info), mVal(val) { }

	const TokenVal& value() const {
		return mVal;
	}

	const TokenInfo* info() const {
		return mInfo;
	}

	void setFlags(size_t flags) {
		mFlags = flags;
	}

	void addFlags(size_t flags) {
		mFlags |= flags;
	}

	size_t getFlags() const {
		return mFlags;
	}

private:
	const TokenInfo* mInfo;
	TokenVal mVal{};
	size_t mFlags{};
};

struct LexerInputArgs {
	Token& token; 
	LexerSource& source; 
	LexerResultInfo& debug;
	int initState = token_none;
};

class Lexer {
public:
	int next(const LexerInputArgs& args);
	int peek(const LexerInputArgs& args);
	void addSwitch(TokenID state, TokenSwitch checker);
	
	int callCheckers(const TokenSwitchArgs& info);
	
	const TokenInfo* getTokenInfo(const char* name, bool isDyn);

	const TokenInfo* addStatic(const char* value, const TokenInfo& info);
	const TokenInfo* addDynamic(const char* name, const TokenInfo& info);

	const TokenInfo* getStatic(const char* value);
	const TokenInfo* getDynamic(const char* value);
private:
	TokenCheckerMap mCheckers;
	TokenMap mStaticTokens;
	TokenMap mDynamicTokens;
};

#endif