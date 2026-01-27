#include "LexerSources.hpp"

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