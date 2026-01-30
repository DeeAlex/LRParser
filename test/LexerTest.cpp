#include "Lexer.hpp"
#include "LexerBuilder.hpp"
#include "LexerDefs.hpp"
#include "LexerSources.hpp"
#include <array>
#include <gtest/gtest.h>

TEST(Lexer, ExprTest) {
    StringSource src("1343+ 0.434 * gffg/4");
	LexerBuilder builder;
	Lexer lexer = builder.withDefaultStates().withStandardOperators().build();
	
	LexerResultInfo resultInfo;
	Token token;

    int i = 0;

    std::array<TokenID, 7> expectedTokens = {
        token_integer,
        token_plus,
        token_real,
        token_mul,
        token_id,
        token_div,
        token_integer
    };

	while(lexer.next({
		.token = token,
		.source = src,
		.debug = resultInfo,
	}) == TKN_OK) {
		EXPECT_EQ(expectedTokens[i], token.info()->id);
        ++i;
    }
}