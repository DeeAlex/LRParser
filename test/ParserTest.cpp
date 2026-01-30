#include "LexerSources.hpp"
#include <gtest/gtest.h>
#include <LexerBuilder.hpp>
#include <ParserBuilder.hpp>
#include <stack>

std::map<TokenID, std::set<TokenID>> firstSets;

enum RuleOpTags {
	RuleOpTags_none,
	RuleOpTags_plus,
	RuleOpTags_minus,
	RuleOpTags_mul,
	RuleOpTags_div,
	RuleOpTags_pow
};

class TestValueStack : public ParserValueStack {
public:
	int pushTerm(const Token& token) {
		if (token.info()->id == token_integer || token.info()->id == token_real) {
			mStack.push(std::stod(token.value()));
		}
		return 0;
	}
    bool pushReduced(const GrammarRule& rule) {
		if (rule.tag == RuleOpTags_plus) {
			double b = mStack.top();
			mStack.pop();
			double a = mStack.top();
			mStack.pop();
			mStack.push(a + b);
		}

		if (rule.tag == RuleOpTags_minus) {
			double b = mStack.top();
			mStack.pop();
			double a = mStack.top();
			mStack.pop();
			mStack.push(a - b);
		}

		if (rule.tag == RuleOpTags_mul) {
			double b = mStack.top();
			mStack.pop();
			double a = mStack.top();
			mStack.pop();
			mStack.push(a * b);
		}

		if (rule.tag == RuleOpTags_div) {
			double b = mStack.top();
			mStack.pop();
			double a = mStack.top();
			mStack.pop();
			mStack.push(a * b);
		}

		if (rule.tag == RuleOpTags_pow) {
			double b = mStack.top();
			mStack.pop();
			double a = mStack.top();
			mStack.pop();
			mStack.push(std::pow(a, b));
		}
		
		return false;
	}
    bool pop() {
		return false;
	}

	double getTop() const {
		return mStack.top();
	}
private:
	std::stack<double> mStack;
};

TEST(Parser, ExprTest) {
    const StrRule grammar[] = {
		{ "S -> E" },
		{ "E -> E + T", RuleOpTags_plus },
		{ "E -> E - T", RuleOpTags_minus }, 
		{ "E -> T" },
		{ "T -> T * P", RuleOpTags_mul },
		{ "T -> T / P", RuleOpTags_div },
		{ "T -> P" },
		{ "P -> F ^ P", RuleOpTags_pow },
		{ "P -> F" },
		{ "F -> int" },
		{ "F -> real" }
	};

	ParserBuilder parserBuilder;

	Parser parser = parserBuilder.initGrammarLexer().loadGrammar(grammar).build();

	StringSource src("5*2+10*5-4^2");
	LexerBuilder builder;
	Lexer lexerTest = builder.withDefaultStates().withStandardOperators().build();
	TestValueStack valueStack;
	LexerResultInfo resultInfo;

	while (ParseStatus_ok == parser.parseNext({
		.lexer = lexerTest,
		.source = src,
		.lexerResInfo = resultInfo,
		.valueStack = valueStack,
		.startState = 0,
	})) {

	}

    EXPECT_EQ(44, valueStack.getTop());
}