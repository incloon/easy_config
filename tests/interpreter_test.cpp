#include "common_test_header.h"
#include <interpreter.hpp>
#include "test_struct.h"
using namespace std;



TEST_CASE("test struct parse")
{
	REQUIRE(args.argc() > 1);

	ezcfg::Interpreter itp(args.argv()[1]);
	REQUIRE(itp);

	TestStr rr;
	itp.parse(rr);

	CHECK(rr.a == 1);
	CHECK(rr.b == 6.4f);
	CHECK(rr.c == 115);
	CHECK(rr.d[0] == 'd');
	CHECK(rr.d[1] == 'e');
	CHECK(rr.e[0] == string("aa"));
	CHECK(rr.e[1] == string("ss"));
	CHECK(rr.f == "hello world");
	CHECK(rr.g.size() == 3);
	CHECK(rr.g[0] == 1);
	CHECK(rr.g[1] == 2);
	CHECK(rr.g[2] == 3);
	CHECK(rr.h.size() == 2);
	CHECK(rr.h[8.6] == 789);
	CHECK(rr.h[9.654] == 568);
	CHECK(rr.def_value1 == 2);
	CHECK(rr.def_value2 == 22);
}

TEST_CASE("expression parse test")
{
	ezcfg::Interpreter itp("(1), (1.5 + 1) * 2; 33 * -33",false);
	double a = itp.parseExpression();
	CHECK(a == ((1), (1.5 + 1) * 2));
	int b = itp.parseExpression();
	CHECK(b == (33 * -33));
}

struct Inner
{
	int x;
	int y = 9;
};

struct Outer
{
	Inner in;
	int z = 3;
};

TEST_CASE("nested struct and trailing defaults")
{
	ezcfg::Interpreter itp("{ .in = { .x = 4 } }", false);
	Outer o;
	itp.parse(o);
	CHECK(o.in.x == 4);
	CHECK(o.in.y == 9);
	CHECK(o.z == 3);
}

TEST_CASE("missing file throws ParseError")
{
	CHECK_THROWS_AS(ezcfg::Interpreter("no_such_file_ezcfg.cfg"), ezcfg::ParseError);
	try
	{
		ezcfg::Interpreter itp("no_such_file_ezcfg.cfg");
		FAIL("should throw");
	}
	catch (const ezcfg::ParseError& e)
	{
		CHECK(e.line() == 1);
		CHECK(e.column() == 1);
		CHECK(e.message() == "Cannot open file");
	}

	ezcfg::Interpreter itp;
	CHECK_FALSE(itp.loadFile("no_such_file_ezcfg.cfg"));
	CHECK_FALSE(itp);
}

TEST_CASE("parse error reports line")
{
	ezcfg::Interpreter itp("1 +\n@", false);
	try
	{
		itp.parseExpression();
		FAIL("should throw");
	}
	catch (const ezcfg::ParseError& e)
	{
		CHECK(e.file() == "string");
		CHECK(e.line() == 2);
		CHECK(e.column() >= 1);
	}
}

TEST_CASE("wrong member name reports location")
{
	ezcfg::Interpreter itp("{ .nope = 1 }", false);
	Inner inner;
	try
	{
		itp.parse(inner);
		FAIL("should throw");
	}
	catch (const ezcfg::ParseError& e)
	{
		CHECK(e.line() == 1);
		CHECK(e.column() >= 1);
		CHECK(e.message().find("Expected identify x") != std::string::npos);
	}
}
