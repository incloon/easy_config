#ifndef TOKEN_INFO__HPP
#define TOKEN_INFO__HPP

namespace ezcfg
{
	enum class Token : unsigned char
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
		REM                 =  '%' ,

		//not support
		COLON               =  ':' ,
		BIT_NOT             =  '~' ,
		LOG_NOT             =  '!' ,
		BIT_AND             =  '&' ,
		BIT_OR              =  '|' ,
		BIT_XOR             =  '^' ,
		INC                 =  128 , //  ++
		DEC,                //  --
		LOG_AND,            //  &&
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
		MACRO_REGISTER,
#endif // COMPILER

		END
	};
} /* namespace: ezcfg */
#endif /* ! TOKEN_INFO__HPP */
