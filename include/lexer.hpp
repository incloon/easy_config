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

namespace ezcfg
{
	class Lexer
	{
	public:
		Lexer()
			: file_name()
			, line(1)
			, current_token(Token::END)
			, token_text()
			, integer_value(0)
		{};

		bool loadFile(const std::string& file)
		{
			auto ifs_ptr = new std::ifstream(file);
			if (!ifs_ptr->is_open())
			{
				delete ifs_ptr;
				return false;
			}
			stream.reset(ifs_ptr);
			file_name = file;
			line = 1;
			return true;
		}

		bool loadSource(const std::string& source)
		{
			if (source.empty())
				return false;

			stream.reset(new std::stringstream(source));
			file_name = "string";
			line = 1;
			return true;
		}

		char escapeSequences()
		{
			if (stream->get() != '\\')
				lexError("Expected the charator \\");
			switch (stream->peek())
			{
			case 'a':
				stream->get();
				return '\x07';
			case 'b':
				stream->get();
				return '\x08';
			case 'f':
				stream->get();
				return '\x0c';
			case 'n':
				stream->get();
				return '\x0a';
			case 'r':
				stream->get();
				return '\x0d';
			case 't':
				stream->get();
				return '\x09';
			case 'v':
				stream->get();
				return '\x0b';
			case 'x':
			{
				stream->get();
				while (stream->peek() == '0') stream->get();
				char current_char = stream->peek();
				std::string result;
				if ((current_char >= 'a' && current_char <= 'f') || (current_char >= 'A' && current_char <= 'F') || (current_char >= '0' && current_char <= '9'))
				{
					result.push_back(stream->get());
					current_char = stream->peek();
				}
				else
					lexError("Expected a hex number");
				if ((current_char >= 'a' && current_char <= 'f') || (current_char >= 'A' && current_char <= 'F') || (current_char >= '0' && current_char <= '9'))
				{
					result.push_back(stream->get());
					current_char = stream->peek();
					if ((current_char >= 'a' && current_char <= 'f') || (current_char >= 'A' && current_char <= 'F') || (current_char >= '0' && current_char <= '9'))
						lexError("Escape charator hex number out of rang!");
				}
				return std::stoi(result, nullptr, 16);
			}
			default:
				stream->get();
				char current_char = stream->peek();
				if (current_char >= '0' && current_char <= '7')
				{
					int result;
					result = stream->get() - '0';
					current_char = stream->peek();
					if (current_char >= '0' && current_char <= '7')
					{
						result = result * 8 + stream->get() - '0';
						current_char = stream->peek();
						if (current_char >= '0' && current_char <= '7')
						{
							result = result * 8 + stream->get() - '0';
							if (result > 255)
								lexError("Escape charator octal number out of rang!");
						}
						return result;
					}
					else
						return result;
				}
				else
					return stream->get();

				break;
			}
		}

		//must consume a charator
		bool skipComment()
		{
			if (stream->get() != '/')
				lexError("Internal error! Expected the charactor /");

			switch (stream->peek())
			{
			case '/':
				stream->ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				++line;
				return true;
			case '*':
				stream->get();
				while (true)
					switch (stream->get())
					{
					case '*':
						if (stream->peek() == '/')
						{
							stream->get();
							return true;
						}
						break;
					case std::ifstream::traits_type::eof():
						lexError("Multiline comment error");
					case '\n':
						++line;
					default:
						break;
					}
			default:
				return false;
			}
		}

		void matchIntegerSuffix()
		{
			char current_char = stream->peek();
			switch (current_char)
			{
			case 'u':
			case 'U':
				stream->get();
				current_char = stream->peek();
				if (current_char == 'l' || current_char == 'L')
					stream->get(), current_char = stream->peek();
				if (current_char == 'l' || current_char == 'L')
					stream->get(), current_char = stream->peek();
				break;
			case 'l':
			case 'L':
				stream->get();
				current_char = stream->peek();
				if (current_char == 'l' || current_char == 'L')
					stream->get(), current_char = stream->peek();
				if (current_char == 'u' || current_char == 'U')
					stream->get(), current_char = stream->peek();
				break;
			default:
				break;
			}

			if ((current_char >= 'a' && current_char <= 'z') || (current_char >= 'A' && current_char <= 'Z') || (current_char >= '0' && current_char <= '9'))
				lexError("Integer suffix error!");
		}

		void matchFloatSuffix()
		{
			char current_char = stream->peek();
			switch (current_char)
			{
			case 'f':
			case 'F':
			case 'l':
			case 'L':
				stream->get();
				current_char = stream->peek();
			default:
				break;
			}

			if ((current_char >= 'a' && current_char <= 'z') || (current_char >= 'A' && current_char <= 'Z') || (current_char >= '0' && current_char <= '9'))
				lexError("Float suffix error!");
		}

		void recognizeCharactor()
		{
			if (stream->get() != '\'')
				lexError("Expected the charactor '");
			switch (stream->peek())
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
				integer_value = stream->get();
				break;
			}
			if (stream->get() != '\'')
				lexError("Expected the charactor '");
		}

		void recognizeSingleString()
		{
			if (stream->peek() != '"')
				lexError("Expected the charactor \"");

			stream->get();
			while (true)
			{
				switch (stream->peek())
				{
				case '"':
					stream->get();
					return;
				case '\n':
					lexError("Received \\n between \"\"");
				case std::ifstream::traits_type::eof():
					lexError("Expected the charactor \"");
				case '\\':
					token_text.push_back(escapeSequences());
					if (token_text.back() == '\n')
						token_text.pop_back();
				default:
					token_text.push_back(stream->get());
					break;
				}
			}
		}

		void recognizeMultiString()
		{
			if (stream->peek() != '"')
				lexError("Expected the charactor \"");
			while (true)
				switch (stream->peek())
				{
				case '"':
					recognizeSingleString();
				case '\n':
					++line;
				case '\t':
				case ' ':
					stream->get();
					break;
				case '/':    // / // /*
					if (!skipComment())
					{
						if(!stream->unget())
							lexError("Internal error! stream unget fault");
						return;
					}
					break;

				default:
					return;
					break;
				}
		}

		void recognizeID()
		{
			char current_char = stream->peek();
			if ((current_char >= 'a' && current_char <= 'z') || (current_char >= 'A' && current_char <= 'Z') || (current_char == '_'))
			{
				token_text.push_back(stream->get());
				current_char = stream->peek();
			}
			else
				lexError("Expected a identity");
			while ((current_char >= 'a' && current_char <= 'z') || (current_char >= 'A' && current_char <= 'Z') || (current_char >= '0' && current_char <= '9') || (current_char == '_'))
			{
				token_text.push_back(stream->get());
				current_char = stream->peek();
			}

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
#endif // COMPILER
			else
				current_token = Token::ID;
		}

		void recognizeNum()
		{
			char current_char = stream->peek();
			if (current_char < '0' && current_char > '9')
				lexError("Expected a number");

			bool octal_check = false;
			current_token = Token::INT;
			token_text.push_back(stream->get());
			if (current_char == '0')
			{
				current_char = stream->peek();
				switch (current_char)
				{
				case 'b':
				case 'B':
					integer_value = 0;
					token_text.push_back(stream->get());
					current_char = stream->peek();
					if (current_char == '0' || current_char == '1')
					{
						token_text.push_back(stream->get());
						integer_value = current_char - '0';
						current_char = stream->peek();
					}
					else
						lexError("Expected a binary number");

					while (current_char == '0' || current_char == '1')
					{
						token_text.push_back(stream->get());
						integer_value = integer_value * 2 + (current_char - '0');
						current_char = stream->peek();
					}
					return;
				case 'x':
				case 'X':
					token_text.push_back(stream->get());
					current_char = stream->peek();
					while ((current_char >= 'a' && current_char <= 'f') || (current_char >= 'A' && current_char <= 'F') || (current_char >= '0' && current_char <= '9'))
					{
						token_text.push_back(stream->get());
						current_char = stream->peek();
					}
					if (current_char == '.')
					{
						current_token = Token::FLOAT;
						token_text.push_back(stream->get());
						current_char = stream->peek();
						while ((current_char >= 'a' && current_char <= 'f') || (current_char >= 'A' && current_char <= 'F') || (current_char >= '0' && current_char <= '9'))
						{
							token_text.push_back(stream->get());
							current_char = stream->peek();
						}
					}

					if (current_char == 'p' || current_char == 'P')
					{
						current_token = Token::FLOAT;
						token_text.push_back(stream->get());
						current_char = stream->peek();
						if (current_char == '-' || current_char == '+')
						{
							token_text.push_back(stream->get());
							current_char = stream->peek();
						}
						if ((current_char >= 'a' && current_char <= 'f') || (current_char >= 'A' && current_char <= 'F') || (current_char >= '0' && current_char <= '9'))
						{
							token_text.push_back(stream->get());
							current_char = stream->peek();
						}
						else
							lexError("Expected a hex number");

						while ((current_char >= 'a' && current_char <= 'f') || (current_char >= 'A' && current_char <= 'F') || (current_char >= '0' && current_char <= '9'))
						{
							token_text.push_back(stream->get());
							current_char = stream->peek();
						}
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

			current_char = stream->peek();
			while (current_char >= '0' && current_char <= '9')
			{
				token_text.push_back(stream->get());
				current_char = stream->peek();
			}
			if (current_char == '.')
			{
				octal_check = false;
				current_token = Token::FLOAT;
				token_text.push_back(stream->get());
				current_char = stream->peek();
				while (current_char >= '0' && current_char <= '9')
				{
					token_text.push_back(stream->get());
					current_char = stream->peek();
				}
			}

			if (current_char == 'e' || current_char == 'E')
			{
				octal_check = false;
				current_token = Token::FLOAT;
				token_text.push_back(stream->get());
				current_char = stream->peek();
				if (current_char == '-' || current_char == '+')
				{
					token_text.push_back(stream->get());
					current_char = stream->peek();
				}
				if (current_char >= '0' && current_char <= '9')
				{
					token_text.push_back(stream->get());
					current_char = stream->peek();
				}
				else
					lexError("Expected a number");

				while (current_char >= '0' && current_char <= '9')
				{
					token_text.push_back(stream->get());
					current_char = stream->peek();
				}
			}

			if (octal_check)
			{
				for (char c : token_text)
					if (c > '7')
						lexError("Expected a octal number");
			}
			if (current_token == Token::INT)
				integer_value = std::stoull(token_text);
			else if (current_token == Token::FLOAT)
				float_value = std::stod(token_text);
		}

		Token next()
		{
			token_text.clear();
			while (true)
			{
				switch (stream->peek())
				{
				case ':':    //  : ::
					stream->get();
					if (stream->peek() == ':')
					{
						current_token = Token::SCOPE;
						stream->get();
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
					stream->get();
					if (stream->peek() == '<')
					{
						current_token = Token::BIT_L_SHIFT;
						stream->get();
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
					stream->ignore(std::numeric_limits<std::streamsize>::max(), '\n');
					++line;
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
					current_token = static_cast<Token>(stream->get());
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

				case '*':

				case '~':
				case '!':
				case '&':    //  & (&&)
				case '|':    //  | (||)
				case '^':
					std::cout << "Lexer: Current symbol not support! line: " << __LINE__ << std::endl;
					exit(-1);

				case '\n':
					++line;
				case '\t':
				case ' ':
					stream->get();
					break;

				case std::ifstream::traits_type::eof():
					current_token = Token::END;
					return current_token;
					break;

				default:
					char current_char = stream->peek();
					if ((current_char >= 'a' && current_char <= 'z') || (current_char >= 'A' && current_char <= 'Z') || (current_char == '_'))
					{
						recognizeID();
						return current_token;
					}

					if (current_char >= '0' && current_char <= '9')
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

					lexError(std::string("Undefine symbol : ") + current_char);

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
			return stream->good();
		}

		void syntaxError(const std::string& info)
		{
			std::cerr << file_name << " : " << line << " : " << "Syntax error: " << info << std::endl;
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

		void printTokenContent()
		{
			if (current_token == Token::FLOAT)
				std::cout << float_value << std::flush;
			else if (current_token == Token::INT)
				std::cout << integer_value << std::flush;
			else
				std::cout << token_text << std::flush;
		}

		~Lexer()
		{

		}

	private:
		void lexError(const std::string& info)
		{
			std::cerr << file_name << ": " << line << ": " << "Lexical error: " << info << std::endl;
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
#endif // COMPILER
			};

			auto iter = reflex.find(t);
			if (iter != reflex.end())
				return iter->second;
			else
				return std::string(1, static_cast<char>(t));
		}

		std::string file_name;
		std::unique_ptr<std::istream> stream;

		size_t line;
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
