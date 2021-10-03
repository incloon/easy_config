#include "common_test_header.h"
#include <interpreter.hpp>

TEST_CASE("lex")
{
	REQUIRE(args.argc() > 1);

	ezcfg::Lexer lex;
	lex.loadFile(args.argv()[1]);

	REQUIRE(lex);
	REQUIRE(lex.next() == ezcfg::Token::SCOPE);
	REQUIRE(lex.next() == ezcfg::Token::L_ANGLE_BRACKET);
	REQUIRE(lex.next() == ezcfg::Token::L_BRACE);
	REQUIRE(lex.next() == ezcfg::Token::L_BRACE);
	REQUIRE(lex.next() == ezcfg::Token::R_BRACE);
	REQUIRE(lex.next() == ezcfg::Token::R_BRACE);
	REQUIRE(lex.next() == ezcfg::Token::L_BRACKET);
	REQUIRE(lex.next() == ezcfg::Token::L_BRACKET);
	REQUIRE(lex.next() == ezcfg::Token::R_BRACKET);
	REQUIRE(lex.next() == ezcfg::Token::R_BRACKET);
	REQUIRE(lex.next() == ezcfg::Token::L_PARENTHESIS);
	REQUIRE(lex.next() == ezcfg::Token::L_PARENTHESIS);
	REQUIRE(lex.next() == ezcfg::Token::R_PARENTHESIS);
	REQUIRE(lex.next() == ezcfg::Token::R_PARENTHESIS);
	REQUIRE(lex.next() == ezcfg::Token::R_ANGLE_BRACKET);
	REQUIRE(lex.next() == ezcfg::Token::R_ANGLE_BRACKET);
	REQUIRE(lex.next() == ezcfg::Token::DOT);
	REQUIRE(lex.next() == ezcfg::Token::DOT);
	REQUIRE(lex.next() == ezcfg::Token::COMMA);
	REQUIRE(lex.next() == ezcfg::Token::COMMA);
	REQUIRE(lex.next() == ezcfg::Token::SEMICOLON);
	REQUIRE(lex.next() == ezcfg::Token::SEMICOLON);
	REQUIRE(lex.next() == ezcfg::Token::EQU);
	REQUIRE(lex.next() == ezcfg::Token::ADD);
	REQUIRE(lex.next() == ezcfg::Token::ADD);
	REQUIRE(lex.next() == ezcfg::Token::SUB);
	REQUIRE(lex.next() == ezcfg::Token::SUB);

	REQUIRE(lex.next() == ezcfg::Token::INT);
	REQUIRE(lex.getIntegetValue() == 'a');
	REQUIRE(lex.next() == ezcfg::Token::INT);
	REQUIRE(lex.getIntegetValue() == 'n');
	REQUIRE(lex.next() == ezcfg::Token::INT);
	REQUIRE(lex.getIntegetValue() == '*');
	REQUIRE(lex.next() == ezcfg::Token::INT);
	REQUIRE(lex.getIntegetValue() == '\n');
	REQUIRE(lex.next() == ezcfg::Token::INT);
	REQUIRE(lex.getIntegetValue() == '\075');
	REQUIRE(lex.next() == ezcfg::Token::INT);
	REQUIRE(lex.getIntegetValue() == '\55');
	REQUIRE(lex.next() == ezcfg::Token::INT);
	REQUIRE(lex.getIntegetValue() == '\x0f');
	REQUIRE(lex.next() == ezcfg::Token::STR);
	REQUIRE(lex.getTokenText() == "this is string\n\"esc\"");

	REQUIRE(lex.next() == ezcfg::Token::INT);
	REQUIRE(lex.getIntegetValue() == 1);
	REQUIRE(lex.getTokenText() == "true");
	REQUIRE(lex.next() == ezcfg::Token::INT);
	REQUIRE(lex.getIntegetValue() == 0);
	REQUIRE(lex.getTokenText() == "false");

	REQUIRE(lex.next() == ezcfg::Token::ID);
	REQUIRE(lex.getTokenText() == "_0identify");

	REQUIRE(lex.next() == ezcfg::Token::INT);
	REQUIRE(lex.getIntegetValue() == 0666);
	REQUIRE(lex.next() == ezcfg::Token::INT);
	REQUIRE(lex.getIntegetValue() == 96354);
	REQUIRE(lex.next() == ezcfg::Token::INT);
	REQUIRE(lex.getIntegetValue() == 0xffe55);
	REQUIRE(lex.next() == ezcfg::Token::FLOAT);
	REQUIRE(lex.getFloatValue() == 3.6);
	REQUIRE(lex.next() == ezcfg::Token::FLOAT);
	REQUIRE(lex.getFloatValue() == 5.999);
	REQUIRE(lex.next() == ezcfg::Token::FLOAT);
	REQUIRE(lex.getFloatValue() == 0xff.55ap-06);

	REQUIRE(lex.next() == ezcfg::Token::END);
}
