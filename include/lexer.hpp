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
			class FormatFilterStream
			{
			public:
				FormatFilterStream(size_t& line, const std::unique_ptr<std::istream>& stream)
					: current_charator{ 0 }
					, line{ line }
					, stream{ stream }
				{}

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
								if (stream->peek() != '\n')
								{
									stream->unget();
									return temp;
								}
							case '\n':
								stream->get();
								++line;
								break;
							case std::ifstream::traits_type::eof():
								current_charator = std::ifstream::traits_type::eof();
								return temp;
							default:
								return temp;
							}
							break;
						case '\r'://fallthrough
							if (stream->peek() == '\n')
							{
								stream->get();
								current_charator = '\n';
							}
							else
								return temp;
						case '\n':
							++line;
						default:
							return temp;
						}
				}

				char peek() const
				{
					return current_charator;
				}

			private:
				char current_charator;
				size_t& line;
				const std::unique_ptr<std::istream>& stream;
			};

			class CommentFilterStream
			{
			public:
				CommentFilterStream(const std::string& file_name, FormatFilterStream& stream)
					: current_charator{ 0 }
					, file_name{ file_name }
					, stream{ stream }
				{}

				char get()
				{
					char temp = current_charator;
					while (true)
						switch (current_charator = stream.get())
						{
						case '/':
							switch (stream.peek())
							{
							case '/':
								while (stream.get() != '\n');
								current_charator = ' ';
								return temp;
							case '*':
								stream.get();
								while (true)
									switch (stream.get())
									{
									case '*':
										if (stream.peek() == '/')
										{
											stream.get();
											current_charator = ' ';
											return temp;
										}
										break;
									case std::ifstream::traits_type::eof():
										//std::cerr << file_name << ": " << line << ": " << "Lexical error: Multiline comment error" << std::endl;
										exit(-1);
									default:
										break;
									}
							default:
								break;
							}
							return temp;
						default:
							return temp;
						}
				}

				char peek() const
				{
					return current_charator;
				}

			private:
				char current_charator;
				const std::string& file_name;
				FormatFilterStream& stream;
			};

		public:
			FilterStream(const std::string& file_name)
				: line{ 1 }
				, file_name{ file_name }
				, base_stream{ nullptr }
				, format_filter_stream{ line, base_stream }
				, comment_filter_stream{ file_name, format_filter_stream }
			{}

			bool loadFile(const std::string& file)
			{
				auto ifs_ptr = new std::ifstream(file);
				if (!ifs_ptr->is_open())
				{
					delete ifs_ptr;
					return false;
				}
				base_stream.reset(ifs_ptr);
				line = 1;
				format_filter_stream.get();
				comment_filter_stream.get();
				return true;
			}

			bool loadSource(const std::string& source)
			{
				if (source.empty())
					return false;

				base_stream.reset(new std::stringstream(source));
				line = 1;
				format_filter_stream.get();
				comment_filter_stream.get();
				return true;
			}

			inline char get()
			{
				return comment_filter_stream.get();
			}

			inline char peek()
			{
				return comment_filter_stream.peek();
			}

			inline char getRaw()
			{
				return base_stream->get();
			}

			inline size_t getLineNum()
			{
				return line;
			}

			explicit operator bool() const
			{
				return comment_filter_stream.peek() != std::ifstream::traits_type::eof();
			}

		private:
			size_t line;
			const std::string& file_name;
			std::unique_ptr<std::istream> base_stream;
			FormatFilterStream format_filter_stream;
			CommentFilterStream comment_filter_stream;
		};

		template<bool (Lexer::*charSet)()>
		inline void matchCS(std::string& seq)
		{ while ((this->*charSet)()) seq.push_back(stream.get()); }

		template<bool (Lexer::*charSet)()>
		void recognizeCS(const std::string& info, std::string& seq)
		{
			if ((this->*charSet)())
				do seq.push_back(stream.get());
				while ((this->*charSet)());
			else lexError(info);
		}

		inline bool binarySet()
		{ return stream.peek() == '0' || stream.peek() == '1'; }

		inline bool octalSet()
		{ return stream.peek() >= '0' && stream.peek() <= '7'; }

		inline bool decimalSet()
		{ return stream.peek() >= '0' && stream.peek() <= '9'; }

		inline bool hexadecimalSet()
		{ return stream.peek() >= 'a' && stream.peek() <= 'f' || stream.peek() >= 'A' && stream.peek() <= 'F' || stream.peek() >= '0' && stream.peek() <= '9'; }

		inline bool identifierSet()
		{ return stream.peek() >= 'a' && stream.peek() <= 'z' || stream.peek() >= 'A' && stream.peek() <= 'Z' || stream.peek() >= '0' && stream.peek() <= '9' || stream.peek() == '_'; }

		inline void recognizeBinaryCS(std::string& seq)
		{ recognizeCS<&Lexer::binarySet>("Expected a binary number", seq); }

		inline void recognizeOctalCS(std::string& seq)
		{ recognizeCS<&Lexer::octalSet>("Expected a octal number", seq); }

		inline void recognizeDecimalCS(std::string& seq)
		{ recognizeCS<&Lexer::decimalSet>("Expected a decimal number", seq); }

		inline void recognizeHexadecimalCS(std::string& seq)
		{ recognizeCS<&Lexer::hexadecimalSet>("Expected a hexadecimal number", seq); }

		char charToNum(char c)
		{
			if (c >= '0' && c <= '9')
				return c - '0';
			if (c >= 'A' && c <= 'Z')
				return c - 'A' + 10;
			if (c >= 'a' && c <= 'z')
				return c - 'a' + 10;

			lexError("Internal error: charToNum");
		}

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
				recognizeHexadecimalCS(result);
				if (result.size() == 1)
					return charToNum(result.back());
				else
					return charToNum(*(result.end() - 2)) * 16 + charToNum(result.back());
			}
			default:
				if (octalSet())
				{
					int result;
					result = stream.get() - '0';
					if (octalSet())
					{
						result = result * 8 + stream.get() - '0';
						if (octalSet())
						{
							result = result * 8 + stream.get() - '0';
							if (result > 255)
								lexError("Escape charator octal number out of rang!");
						}
					}
					return result;
				}
				else
					return stream.get();

				break;
			}
		}

		bool matchIntegerSuffix()
		{
			bool res = true;
			switch (stream.peek())
			{
			case 'u':
			case 'U':
				res = false;
				token_text.push_back(stream.get());
				if (stream.peek() == 'l')
				{
					token_text.push_back(stream.get());
					if (stream.peek() == 'l')
						token_text.push_back(stream.get());
				}
				else if (stream.peek() == 'L')
				{
					token_text.push_back(stream.get());
					if (stream.peek() == 'L')
						token_text.push_back(stream.get());
				}
				break;
			case 'l':
				token_text.push_back(stream.get());
				if (stream.peek() == 'l')
					token_text.push_back(stream.get());
				if (stream.peek() == 'u' || stream.peek() == 'U')
					res = false, token_text.push_back(stream.get());
				break;
			case 'L':
				stream.get();
				if (stream.peek() == 'L')
					token_text.push_back(stream.get());
				if (stream.peek() == 'u' || stream.peek() == 'U')
					res = false, token_text.push_back(stream.get());
				break;
			default:
				break;
			}

			if (identifierSet())
				lexError("Integer suffix error!");

			return res;
		}

		void matchFloatSuffix()
		{
			switch (stream.peek())
			{
			case 'f':
			case 'F':
			case 'l':
			case 'L':
				token_text.push_back(stream.get());
			default:
				break;
			}

			if (identifierSet())
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
				number = escapeSequences();
				break;
			default:
				number = stream.get();
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
				case '\t':
				case '\v':
				case '\f':
				case '\n':
				case ' ':
					stream.get();
					break;
				default:
					return;
					break;
				}
		}

		void recognizeID()
		{
			if (stream.peek() >= 'a' && stream.peek() <= 'z' || stream.peek() >= 'A' && stream.peek() <= 'Z' || stream.peek() == '_')
				do token_text.push_back(stream.get());
				while (identifierSet());
			else
				lexError("Expected a identity");

			if (token_text == "true")
			{
				current_token = Token::INT;
				number = true;
			}
			else if (token_text == "false")
			{
				current_token = Token::INT;
				number = false;
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

		bool isSigned(unsigned long long v)
		{
			if (v > std::numeric_limits<int>::max() && v <= std::numeric_limits<unsigned int>::max())
				return false;
			else if (v > std::numeric_limits<long>::max() && v <= std::numeric_limits<unsigned long>::max())
				return false;
			else if (v > std::numeric_limits<long long>::max() && v <= std::numeric_limits<unsigned long long>::max())
				return false;
			return true;
		}

		void recognizeNum()
		{
			if (!decimalSet())
				lexError("Expected a number");

			char* discarded_value;
			bool octal_check = false;
			current_token = Token::INT;
			token_text.push_back(stream.peek());
			if (stream.get() == '0')
			{
				switch (stream.peek())
				{
				case 'b':
				case 'B':
					token_text.push_back(stream.get());
					recognizeBinaryCS(token_text);
					if (token_text.size() - 2 > CHAR_BIT * sizeof(size_t))
						std::_Xout_of_range("stoull argument out of range");
					else
					{
						size_t temp = std::strtoull(token_text.c_str() + 2, &discarded_value, 2);
						number = IntegerT{ temp ,isSigned(temp) && matchIntegerSuffix() };
					}
					return;
				case 'x':
				case 'X':
					token_text.push_back(stream.get());
					matchCS<&Lexer::hexadecimalSet>(token_text);
					if (stream.peek() != '.')
					{
						if (token_text.size() - 2 > CHAR_BIT * sizeof(size_t) / 4)
							std::_Xout_of_range("stoull argument out of range");
						else
						{
							size_t temp = std::strtoull(token_text.c_str(), &discarded_value, 16);
							number = IntegerT{ temp ,isSigned(temp) && matchIntegerSuffix() };
						}
					}
					else
					{
						current_token = Token::FLOAT;
						token_text.push_back(stream.get());
						matchCS<&Lexer::hexadecimalSet>(token_text);
						if (token_text.size() < 4)
							lexError("Expected a hexadecimal number");

						if (stream.peek() == 'p' || stream.peek() == 'P')
						{
							token_text.push_back(stream.get());
							if (stream.peek() == '-' || stream.peek() == '+')
								token_text.push_back(stream.get());
							recognizeHexadecimalCS(token_text);
						}
						number = std::stod(token_text);
						matchFloatSuffix();
					}
					return;
				default:
					octal_check = true;
					break;
				}
			}

			matchCS<&Lexer::decimalSet>(token_text);
			if (stream.peek() != '.')
			{
				if (octal_check)
				{
					for (char c : token_text)
						if (c > '7')
							lexError("Expected a octal number");
					size_t temp = std::stoull(token_text, nullptr, 8);
					number = IntegerT{ temp ,isSigned(temp) && matchIntegerSuffix() };
				}
				else
					number = IntegerT{ std::stoull(token_text) ,matchIntegerSuffix() };
			}
			else
			{
				current_token = Token::FLOAT;
				token_text.push_back(stream.get());
				matchCS<&Lexer::decimalSet>(token_text);
				if (stream.peek() == 'e' || stream.peek() == 'E')
				{
					token_text.push_back(stream.get());
					if (stream.peek() == '-' || stream.peek() == '+')
						token_text.push_back(stream.get());
					recognizeDecimalCS(token_text);
				}
				number = std::stod(token_text);
				matchFloatSuffix();
			}
		}

		void recognizeDotBeginFloat()
		{
			recognizeDecimalCS(token_text);
			if (stream.peek() == 'e' || stream.peek() == 'E')
			{
				token_text.push_back(stream.get());
				if (stream.peek() == '-' || stream.peek() == '+')
					token_text.push_back(stream.get());
				recognizeDecimalCS(token_text);
			}
			number = std::stod(token_text);
		}

	public:
		Lexer()
			: file_name{}
			, stream{ file_name }
			, current_token{ Token::END }
			, token_text{}
			, number{ 0 }
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
						stream.get();
						return current_token = Token::SCOPE;
					}
					else
					{
						std::cout << "Lexer: Current symbol not support! line: " << __LINE__ << std::endl;
						exit(-1);
						return current_token = Token::COLON;
					}
				case '.':
					token_text.push_back(stream.get());
					if (decimalSet())
					{
						recognizeDotBeginFloat();
						return current_token = Token::FLOAT;
					}
					else
						return current_token = Token::DOT;

				case '#':
					while (stream.get() != '\n');
					break;
				case '{':
				case '}':
				case '[':
				case ']':
				case '(':
				case ')':
				case '<':    //  < <<
				case '>':
				case ',':
				case ';':
				case '=':    //  = (==)
				case '+':
				case '-':
				case '*':
				case '/':
				case '%':
					return current_token = static_cast<Token>(stream.get());

				case '\'':
					recognizeCharactor();
					return current_token = Token::INT;

				case '"':
					recognizeMultiString();
					return current_token = Token::STR;

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
					return current_token = Token::END;
					break;

				default:
					if ((stream.peek() >= 'a' && stream.peek() <= 'z') || (stream.peek() >= 'A' && stream.peek() <= 'Z') || (stream.peek() == '_'))
					{
						recognizeID();
						return current_token;
					}

					if (decimalSet())
					{
						recognizeNum();
						return current_token;
					}

					lexError(std::string("Undefine symbol : ") + stream.peek());

					break;
				}
			}
		}

		ArithmeticT getNumber()
		{
			if (current_token == Token::INT || current_token == Token::FLOAT)
				return number;

			syntaxError("Get float value of a not number token");
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
			case Token::FLOAT:
				std::cerr << " value is " << number;
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
		ArithmeticT number;
	};
} /* namespace: ezcfg */
#endif /* !__LEXER_HPP__ */
