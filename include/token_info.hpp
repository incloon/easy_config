#ifndef __TOKEN_INFO_HPP__
#define __TOKEN_INFO_HPP__

namespace ezcfg
{
	enum class Token : unsigned char
	{
		SCOPE,              //  ::
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
		
		//not support
		COLON               =  ':' ,
		BIT_NOT             =  '~' ,
		LOG_NOT             =  '!' ,
		BIT_L_SHIFT,        //  <<
		BIT_AND             =  '&' ,
		LOG_AND,            //  &&
		BIT_OR              =  '|' ,
		LOG_OR,             //  ||
		BIT_XOR             =  '^' ,
		
		END
	};
} /* namespace: ezcfg */
#endif /* !__TOKEN_INFO_HPP__ */
