#ifndef __LEXER_HPP__
#define __LEXER_HPP__

#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <string>
#include <memory>
#include <map>
#include <token_info.hpp>
#include <arithmetic_type.hpp>

namespace ezcfg
{
	class Lexer
	{
		class FilterStream
		{
		public:
			FilterStream()
				: current_charator{ 0 }
				, line{ 1 }
				, stream{}
			{}

			bool loadFile(const std::string& file)
			{
				auto ifs_ptr = new std::ifstream(file);
				if (!ifs_ptr->is_open())
				{
					delete ifs_ptr;
					return false;
				}
				stream.reset(ifs_ptr);
				line = 1;
				get();
				return true;
			}

			bool loadSource(const std::string& source)
			{
				if (source.empty())
					return false;

				stream.reset(new std::stringstream(source));
				line = 1;
				return true;
			}

			char get()
			{
				char temp = current_charator;
				while (true)
					switch (current_charator = stream->get())
					{
					case '\\':
						switch (stream->peek())
						{
						case '\r':
							stream->get();
							++line;
							if (stream->peek() != '\n')
								break;
						case '\n':
							stream->get();
							break;
						case std::ifstream::traits_type::eof():
							current_charator = std::ifstream::traits_type::eof();
							return temp;
						default:
							current_charator = '\\';
							return temp;
						}
						break;
					case '\r':
						if (stream->peek() == '\n')
							stream->get();
						else
							return temp;
					case '\n':
						current_charator = '\n';
						++line;
					default:
						return temp;
					}
			}

			char peek()
			{
				return current_charator;
			}

			void ignore(char sentinel)
			{
				while (get() != sentinel)
					if (peek() == std::ifstream::traits_type::eof())
						return;
			}

			size_t getLineNum()
			{
				return line;
			}

			explicit operator bool() const
			{
				return stream->good();
			}

		private:
			char current_charator;
			size_t line;
			std::unique_ptr<std::istream> stream;
		};

		char escapeSequences()
		{
			if (stream.get() != '\\')
				lexError("Expected the charator \\");
			switch (stream.peek())
			{
			case 'a':
				stream.get();
				return '\x07';
			case 'b':
				stream.get();
				return '\x08';
			case 'f':
				stream.get();
				return '\x0c';
			case 'n':
				stream.get();
				return '\x0a';
			case 'r':
				stream.get();
				return '\x0d';
			case 't':
				stream.get();
				return '\x09';
			case 'v':
				stream.get();
				return '\x0b';
			case 'x':
			{
				stream.get();
				while (stream.peek() == '0') stream.get();
				std::string result;
				if ((stream.peek() >= 'a' && stream.peek() <= 'f') || (stream.peek() >= 'A' && stream.peek() <= 'F') || (stream.peek() >= '0' && stream.peek() <= '9'))
					result.push_back(stream.get());
				else
					lexError("Expected a hex number");
				if ((stream.peek() >= 'a' && stream.peek() <= 'f') || (stream.peek() >= 'A' && stream.peek() <= 'F') || (stream.peek() >= '0' && stream.peek() <= '9'))
				{
					result.push_back(stream.get());
					if ((stream.peek() >= 'a' && stream.peek() <= 'f') || (stream.peek() >= 'A' && stream.peek() <= 'F') || (stream.peek() >= '0' && stream.peek() <= '9'))
						lexError("Escape charator hex number out of rang!");
				}
				return std::stoi(result, nullptr, 16);
			}
			default:
				if (stream.peek() >= '0' && stream.peek() <= '7')
				{
					int result;
					result = stream.get() - '0';
					if (stream.peek() >= '0' && stream.peek() <= '7')
					{
						result = result * 8 + stream.get() - '0';
						if (stream.peek() >= '0' && stream.peek() <= '7')
						{
							result = result * 8 + stream.get() - '0';
							if (result > 255)
								lexError("Escape charator octal number out of rang!");
						}
						return result;
					}
					else
						return result;
				}
				else
					return stream.get();

				break;
			}
		}

		//must consume a charator
		bool skipComment()
		{
			if (stream.get() != '/')
				lexError("Internal error! Expected the charactor /");

			switch (stream.peek())
			{
			case '/':
				stream.ignore('\n');
				return true;
			case '*':
				stream.get();
				while (true)
					switch (stream.get())
					{
					case '*':
						if (stream.peek() == '/')
						{
							stream.get();
							return true;
						}
						break;
					case std::ifstream::traits_type::eof():
						lexError("Multiline comment error");
					default:
						break;
					}
			default:
				return false;
			}
		}

		void matchIntegerSuffix()
		{
			switch (stream.peek())
			{
			case 'u':
			case 'U':
				stream.get();
				if (stream.peek() == 'l' || stream.peek() == 'L')
					stream.get();
				if (stream.peek() == 'l' || stream.peek() == 'L')
					stream.get();
				break;
			case 'l':
			case 'L':
				stream.get();
				if (stream.peek() == 'l' || stream.peek() == 'L')
					stream.get();
				if (stream.peek() == 'u' || stream.peek() == 'U')
					stream.get();
				break;
			default:
				break;
			}

			if ((stream.peek() >= 'a' && stream.peek() <= 'z') || (stream.peek() >= 'A' && stream.peek() <= 'Z') || (stream.peek() >= '0' && stream.peek() <= '9'))
				lexError("Integer suffix error!");
		}

		void matchFloatSuffix()
		{
			switch (stream.peek())
			{
			case 'f':
			case 'F':
			case 'l':
			case 'L':
				stream.get();
			default:
				break;
			}

			if ((stream.peek() >= 'a' && stream.peek() <= 'z') || (stream.peek() >= 'A' && stream.peek() <= 'Z') || (stream.peek() >= '0' && stream.peek() <= '9'))
				lexError("Float suffix error!");
		}

		void recognizeCharactor()
		{
			if (stream.get() != '\'')
				lexError("Expected the charactor '");
			switch (stream.peek())
			{
			case '\'':
				lexError("Void character");
			case '\n':
				lexError("Received \\n between ''");
			case std::ifstream::traits_type::eof():
				lexError("Expected the charactor '");
			case '\\':
				integer_value = escapeSequences();
				break;
			default:
				integer_value = stream.get();
				break;
			}
			if (stream.get() != '\'')
				lexError("Expected the charactor '");
		}

		void recognizeSingleString()
		{
			if (stream.get() != '"')
				lexError("Expected the charactor \"");

			while (true)
			{
				switch (stream.peek())
				{
				case '"':
					stream.get();
					return;
				case '\n':
					lexError("Received \\n between \"\"");
				case std::ifstream::traits_type::eof():
					lexError("Expected the charactor \"");
				case '\\':
					token_text.push_back(escapeSequences());
					break;
				default:
					token_text.push_back(stream.get());
					break;
				}
			}
		}

		void recognizeMultiString()
		{
			if (stream.peek() != '"')
				lexError("Expected the charactor \"");
			while (true)
				switch (stream.peek())
				{
				case '"':
					recognizeSingleString();
					break;
				case '\n':
				case '\r':
				case '\t':
				case ' ':
					stream.get();
					break;
				case '/':    // / // /*
					if (!skipComment())
						syntaxError("Unexpected token DIV");
					break;
				default:
					return;
					break;
				}
		}

		void recognizeID()
		{
			if ((stream.peek() >= 'a' && stream.peek() <= 'z') || (stream.peek() >= 'A' && stream.peek() <= 'Z') || (stream.peek() == '_'))
				token_text.push_back(stream.get());
			else
				lexError("Expected a identity");
			while ((stream.peek() >= 'a' && stream.peek() <= 'z') || (stream.peek() >= 'A' && stream.peek() <= 'Z') || (stream.peek() >= '0' && stream.peek() <= '9') || (stream.peek() == '_'))
				token_text.push_back(stream.get());

			if (token_text == "true")
			{
				current_token = Token::INT;
				integer_value = 1;
			}
			else if (token_text == "false")
			{
				current_token = Token::INT;
				integer_value = 0;
			}
#ifdef COMPILER
			else if (token_text == "namespace")
				current_token = Token::NAMESPACE;
			else if (token_text == "struct")
				current_token = Token::STRUCT;
			else if (token_text == "enum")
				current_token = Token::ENUM;
			else if (token_text == "const")
				current_token = Token::CONSTANT;
			else if (token_text == "EZCFG_REGISTER_STRUCT")
				current_token = Token::MACRO_REGISTER;
#endif // COMPILER
			else
				current_token = Token::ID;
		}

		void recognizeNum()
		{
			if (stream.peek() < '0' && stream.peek() > '9')
				lexError("Expected a number");

			bool octal_check = false;
			current_token = Token::INT;
			token_text.push_back(stream.peek());
			if (stream.get() == '0')
			{
				switch (stream.peek())
				{
				case 'b':
				case 'B':
					integer_value = 0;
					token_text.push_back(stream.get());
					if (stream.peek() == '0' || stream.peek() == '1')
					{
						integer_value = stream.peek() - '0';
						token_text.push_back(stream.get());
					}
					else
						lexError("Expected a binary number");

					while (stream.peek() == '0' || stream.peek() == '1')
					{
						integer_value = integer_value * 2 + (stream.peek() - '0');
						token_text.push_back(stream.get());
					}
					return;
				case 'x':
				case 'X':
					token_text.push_back(stream.get());
					while ((stream.peek() >= 'a' && stream.peek() <= 'f') || (stream.peek() >= 'A' && stream.peek() <= 'F') || (stream.peek() >= '0' && stream.peek() <= '9'))
						token_text.push_back(stream.get());
					if (stream.peek() == '.')
					{
						current_token = Token::FLOAT;
						token_text.push_back(stream.get());
						while ((stream.peek() >= 'a' && stream.peek() <= 'f') || (stream.peek() >= 'A' && stream.peek() <= 'F') || (stream.peek() >= '0' && stream.peek() <= '9'))
							token_text.push_back(stream.get());
					}

					if (stream.peek() == 'p' || stream.peek() == 'P')
					{
						current_token = Token::FLOAT;
						token_text.push_back(stream.get());
						if (stream.peek() == '-' || stream.peek() == '+')
							token_text.push_back(stream.get());
						if ((stream.peek() >= 'a' && stream.peek() <= 'f') || (stream.peek() >= 'A' && stream.peek() <= 'F') || (stream.peek() >= '0' && stream.peek() <= '9'))
							token_text.push_back(stream.get());
						else
							lexError("Expected a hex number");

						while ((stream.peek() >= 'a' && stream.peek() <= 'f') || (stream.peek() >= 'A' && stream.peek() <= 'F') || (stream.peek() >= '0' && stream.peek() <= '9'))
							token_text.push_back(stream.get());
					}
					if (current_token == Token::INT)
						integer_value = std::stoull(token_text, nullptr, 16);
					else if (current_token == Token::FLOAT)
						float_value = std::stod(token_text);
					return;
				default:
					octal_check = true;
					break;
				}
			}

			while (stream.peek() >= '0' && stream.peek() <= '9')
				token_text.push_back(stream.get());
			if (stream.peek() == '.')
			{
				octal_check = false;
				current_token = Token::FLOAT;
				token_text.push_back(stream.get());
				while (stream.peek() >= '0' && stream.peek() <= '9')
					token_text.push_back(stream.get());
			}

			if (stream.peek() == 'e' || stream.peek() == 'E')
			{
				octal_check = false;
				current_token = Token::FLOAT;
				token_text.push_back(stream.get());
				if (stream.peek() == '-' || stream.peek() == '+')
					token_text.push_back(stream.get());
				if (stream.peek() >= '0' && stream.peek() <= '9')
					token_text.push_back(stream.get());
				else
					lexError("Expected a number");

				while (stream.peek() >= '0' && stream.peek() <= '9')
					token_text.push_back(stream.get());
			}

			if (octal_check)
			{
				if (current_token != Token::INT)
					lexError("Internal error: octal transform");

				for (char c : token_text)
					if (c > '7')
						lexError("Expected a octal number");
				integer_value = std::stoull(token_text, nullptr, 8);
			}
			else if (current_token == Token::INT)
				integer_value = std::stoull(token_text);
			else if (current_token == Token::FLOAT)
				float_value = std::stod(token_text);
		}

	public:
		Lexer()
			: file_name{}
			, stream{}
			, current_token{ Token::END }
			, token_text{}
			, integer_value{ 0 }
		{};

		bool loadFile(const std::string& file)
		{
			if (!stream.loadFile(file))
				return false;
			file_name = file;
			return true;
		}

		bool loadSource(const std::string& source)
		{
			if (!stream.loadSource(source))
				return false;
			file_name = "string";
			return true;
		}

		Token next()
		{
			token_text.clear();
			while (true)
			{
				switch (stream.peek())
				{
				case ':':    //  : ::
					stream.get();
					if (stream.peek() == ':')
					{
						current_token = Token::SCOPE;
						stream.get();
						return current_token;
					}
					else
					{
						current_token = Token::COLON;
						std::cout << "Lexer: Current symbol not support! line: " << __LINE__ << std::endl;
						exit(-1);
						return current_token;
					}
				case '<':    //  < <<
					stream.get();
					if (stream.peek() == '<')
					{
						current_token = Token::BIT_L_SHIFT;
						stream.get();
						std::cout << "Lexer: Current symbol not support! line: " << __LINE__ << std::endl;
						exit(-1);
						return current_token;
					}
					else
					{
						current_token = Token::L_ANGLE_BRACKET;
						return current_token;
					}
				case '#':
					stream.ignore('\n');
					break;
				case '{':
				case '}':
				case '[':
				case ']':
				case '(':
				case ')':
				case '>':
				case '.':
				case ',':
				case ';':
				case '=':    //  = (==)
				case '+':
				case '-':
				case '*':
				case '%':
					current_token = static_cast<Token>(stream.get());
					return current_token;

				case '/':    // / // /*
					if (!skipComment())
					{
						current_token = Token::DIV;
						std::cout << "Lexer: Current symbol not support! line: " << __LINE__ << std::endl;
						exit(-1);
						return current_token;
					}
					break;

				case '\'':
					recognizeCharactor();
					current_token = Token::INT;
					return current_token;

				case '"':
					recognizeMultiString();
					current_token = Token::STR;
					return current_token;


				case '~':
				case '!':
				case '&':    //  & (&&)
				case '|':    //  | (||)
				case '^':
				case '?':
					std::cout << "Lexer: Current symbol not support! line: " << __LINE__ << std::endl;
					exit(-1);

				case ' ':
				case '\t':
				case '\v':
				case '\f':
				case '\n':
					stream.get();
					break;

				case std::ifstream::traits_type::eof():
					current_token = Token::END;
					return current_token;
					break;

				default:
					if ((stream.peek() >= 'a' && stream.peek() <= 'z') || (stream.peek() >= 'A' && stream.peek() <= 'Z') || (stream.peek() == '_'))
					{
						recognizeID();
						return current_token;
					}

					if (stream.peek() >= '0' && stream.peek() <= '9')
					{
						recognizeNum();

						if (current_token == Token::INT)
							matchIntegerSuffix();
						else if (current_token == Token::FLOAT)
							matchFloatSuffix();
						else
							lexError("Internal error in function recognizeDecimalNum()");
						return current_token;
					}

					lexError(std::string("Undefine symbol : ") + stream.peek());

					break;
				}
			}
		}

		double getFloatValue()
		{
			if (current_token == Token::FLOAT)
				return float_value;
			else if (current_token == Token::INT)
				return integer_value;

			syntaxError("Get float value of a not number token");
		}

		size_t getIntegetValue()
		{
			if (current_token == Token::INT)
				return integer_value;
			else if (current_token == Token::FLOAT)
				return float_value;

			syntaxError("Get integet value of a not number token");
		}

		Token match(Token t)
		{
			if (current_token == t)
				return next();
			else
				syntaxError(std::string("Expected token ") + tokenToString(t));
		}

		void matchID(const char* id)
		{
			if (current_token == Token::ID && token_text == id)
				next();
			else
				syntaxError(std::string("Expected identify ") + id);
		}

		bool option(Token t)
		{
			if (current_token == t)
			{
				next();
				return true;
			}

			return false;
		}

		Token getToken()
		{
			return current_token;
		}

		const std::string& getTokenText()
		{
			return token_text;
		}

		const std::string& currentFilename()
		{
			return file_name;
		}

		explicit operator bool() const
		{
			return static_cast<bool>(stream);
		}

		[[noreturn]] void syntaxError(const std::string& info)
		{
			std::cerr << file_name << " : " << stream.getLineNum() << " : " << "Syntax error: " << info << std::endl;
			std::cerr << "current token is " << tokenToString(current_token);
			switch (current_token)
			{
			case Token::INT:
				std::cerr << " value is " << integer_value;
				break;
			case Token::FLOAT:
				std::cerr << " value is " << float_value;
				break;
			case Token::STR:
				std::cerr << " value is " << token_text;
				break;
			default:
				break;
			}
			std::cerr << std::endl;
			exit(-1);
		}

		[[noreturn]] void lexError(const std::string& info)
		{
			std::cerr << file_name << ": " << stream.getLineNum() << ": " << "Lexical error: " << info << std::endl;
			exit(-1);
		}

		std::string tokenToString(Token t)
		{
			static const std::map<Token, std::string> reflex =
			{
				{Token::SCOPE,"::"},
				{Token::INT,  "integer number"},
				{Token::FLOAT,"float number"},
				{Token::STR,"string"},
				{Token::ID,"identify"},
				{Token::BIT_L_SHIFT,"<<"},
				{Token::LOG_AND,"&&"},
				{Token::LOG_OR,"||"},
				{Token::END,"EOF"},
#ifdef COMPILER
				{Token::STRUCT,"struct"},
				{Token::NAMESPACE,"namespace"},
				{Token::ENUM,"enum"},
				{Token::CONSTANT,"const"},
				{Token::MACRO_REGISTER,"EZCFG_REGISTER_STRUCT"},
#endif // COMPILER
			};

			auto iter = reflex.find(t);
			if (iter != reflex.end())
				return iter->second;
			else
				return std::string(1, static_cast<char>(t));
		}

	private:
		FilterStream stream;
		std::string file_name;

		Token current_token;
		std::string token_text;
		union
		{
			size_t integer_value;
			double float_value;
		};
	};
} /* namespace: ezcfg */
#endif /* !__LEXER_HPP__ */
