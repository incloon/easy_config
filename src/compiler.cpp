#define COMPILER
#include <interpreter.hpp>

namespace ezcfg
{
	struct IdentifyInfo
	{
	};

	struct NameScope
	{
		std::map<std::string, NameScope*> tree;
		std::map<std::string, IdentifyInfo> member;
		NameScope* father;
	};

	struct NameTree
	{
		NameScope name_tree;
	};

	class Compiler
	{
	public:
		Compiler(const std::string file)
			: stream(new std::ofstream)
		{
			auto ofs_ptr = new std::ofstream(file);
			if (!ofs_ptr->is_open())
			{
				delete ofs_ptr;
				exit(-1);
			}
			stream.reset(ofs_ptr);
		}

		void loadFile(const std::string& file)
		{
			if (!lex.loadFile(file))
				exit(-1);
			lex.next();
		}

		void loadSource(const std::string& source)
		{
			if (!lex.loadSource(source))
				exit(-1);
			lex.next();
		}

		void temp()
		{
			lex.match(Token::L_ANGLE_BRACKET);
			type_name.push_back('<');
			while(true)
				switch (lex.getToken())
				{
				case Token::L_ANGLE_BRACKET:
					temp();
					break;
				case Token::L_BRACKET:
					while (lex.getToken() == Token::L_BRACKET)
					{
						lex.match(Token::L_BRACKET);
						type_name.push_back('[');
						type_name += std::to_string(lex.getIntegetValue());
						lex.match(Token::INT);
						lex.match(Token::R_BRACKET);
						type_name.push_back(']');
					}
					break;
				case Token::ID:
					type_name += lex.getTokenText();
					lex.next();
					break;
				case Token::SCOPE:
					lex.next();
					type_name += "::";
					break;
				case Token::COMMA:
					lex.next();
					type_name += ", ";
					break;
				case Token::R_ANGLE_BRACKET:
					lex.next();
					type_name.push_back('>');
					return;
				default:
					lex.syntaxError("declear type error");
					break;
				}
		}

		bool decl()
		{
			while(true)
				switch (lex.getToken())
				{
				case Token::ID:
					type_name = lex.getTokenText();
					identify.clear();
					if (lex.next() == Token::ID)
					{
						identify = lex.getTokenText();
						while (lex.next() == Token::ID)
						{
							type_name.push_back(' ');
							type_name += identify;
							identify = lex.getTokenText();
						}
					}
					if(lex.getToken() == Token::SCOPE|| lex.getToken() == Token::L_ANGLE_BRACKET)
						type_name += identify;

					while (lex.getToken() == Token::SCOPE)
					{
						lex.next();
						type_name += "::";
						type_name += lex.getTokenText();
						lex.match(Token::ID);
					}
					if (lex.getToken() == Token::L_ANGLE_BRACKET)
						temp();

					if (lex.getToken() == Token::ID)
					{
						identify = lex.getTokenText();
						lex.next();
					}
					while (lex.getToken() == Token::L_BRACKET)
					{
						lex.match(Token::L_BRACKET);
						type_name.push_back('[');
						type_name += std::to_string(lex.getIntegetValue());
						lex.match(Token::INT);
						lex.match(Token::R_BRACKET);
						type_name.push_back(']');
					}
					lex.match(Token::SEMICOLON);
					return true;
				case Token::SEMICOLON:
					lex.next();
					break;
				default:
					return false;
					break;
				}
		}

		void genCode()
		{
			*stream << "lex.match(Token::DOT);\n";
			*stream << "lex.matchID(\"" << identify << "\");\n";
			*stream << "if (!lex.option(Token::EQU) && lex.getToken() != Token::L_BRACE) lex.option(Token::EQU);\n";
			*stream << "parse(data." << identify << ");//" << type_name << "\n";
		}

		void compile()
		{
			*stream << "template<> void ezcfg::Interpreter::parserDispatcher<";
			lex.match(Token::STRUCT);
			*stream << lex.getTokenText() << ">(" << lex.getTokenText() << "& data)\n{\n";
			lex.match(Token::ID);
			lex.match(Token::L_BRACE);
			*stream << "lex.match(Token::L_BRACE);\n";

			if (decl())
			{
				genCode();
				while (decl())
				{
					*stream << "lex.match(Token::COMMA);\n";
					genCode();
				}
				*stream << "lex.option(Token::COMMA);\n";
			}

			lex.match(Token::R_BRACE);
			*stream << "lex.match(Token::R_BRACE);\n}";

		}

	private:
		Lexer lex;
		std::string type_name;
		std::string identify;
		std::vector<std::string> current_name;
		NameScope name_tree;
		std::unique_ptr<std::ostream> stream;
	};
}

int main(int argc, char** argv)
{
	std::cout << (ezcfg::Token::R_BRACE == ezcfg::Token::LOG_OR) << std::endl;
	ezcfg::Compiler cp(argv[2]);
	cp.loadFile(argv[1]);
	cp.compile();
	return 0;
}
