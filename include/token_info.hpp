#ifndef __TOKEN_INFO_HPP__
#define __TOKEN_INFO_HPP__

namespace ezcfg
{
	enum class Token : unsigned short
	{
		L_BRACE             =  '{' ,
		R_BRACE             =  '}' ,
		L_BRACKET           =  '[' ,
		R_BRACKET           =  ']' ,
		L_PARENTHESIS       =  '(' ,
		R_PARENTHESIS       =  ')' ,
		L_ANGLE_BRACKET     =  '<' ,
		R_ANGLE_BRACKET     =  '>' ,
		DOT                 =  '.' ,
		COMMA               =  ',' ,
		SEMICOLON           =  ';' ,
		HASH                =  '#' ,

		EQU                 =  '=' ,
		ADD                 =  '+' ,
		SUB                 =  '-' ,
		MUL                 =  '*' ,
		DIV                 =  '/' ,

		//not support
		COLON               =  ':' ,
		BIT_NOT             =  '~' ,
		LOG_NOT             =  '!' ,
		BIT_AND             =  '&' ,
		BIT_OR              =  '|' ,
		BIT_XOR             =  '^' ,
		LOG_AND             =  256 , //  &&
		LOG_OR,             //  ||
		BIT_L_SHIFT,        //  <<
		//end not support

		SCOPE,              //  ::
		INT,                // true false
		FLOAT,
		STR,
		ID,

#ifdef COMPILER
		//keyword
		STRUCT,
		NAMESPACE,
		ENUM,
		CONSTANT,
#endif // COMPILER

		END
	};
} /* namespace: ezcfg */
#endif /* !__TOKEN_INFO_HPP__ */
