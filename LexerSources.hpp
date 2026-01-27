#ifndef LEXERSOURCES_HPP
#define LEXERSOURCES_HPP

#include "Lexer.hpp"

class StringSource : public LexerSource {
public:
	StringSource(const char* val) : mStr(val) { }
	StringSource(const std::string& val) : mStr(val) { }
	StringSource(std::string&& val) : mStr(val) { }

	int peekChar(char& ch) override;
	int nextChar(char& ch) override;
	size_t tell() const override;
	bool seek(size_t pos) override;
private:
	size_t mPos = 0;
	std::string mStr;
};

#endif