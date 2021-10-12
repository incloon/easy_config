#ifndef __INTERPRETER_HPP__
#define __INTERPRETER_HPP__

#include <type_traits>
#include <utility>
#include <lexer.hpp>

#include <string>
#include <array>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

namespace ezcfg
{
	class Interpreter
	{
	public:
		Interpreter(const std::string file)
		{
			lex.loadFile(file);
			lex.next();
		}

		template<typename T>
		void parse(T& data)
		{
			parserDispatcher(data);
			lex.option(Token::SEMICOLON);
		}

		template<typename T, typename... TS>
		void parse(T& data, TS&... datas)
		{
			parserDispatcher(data);
			lex.option(Token::SEMICOLON);
			parse(datas...);
		}

		template<typename T>
		void parseExpression(T& num)
		{
			static_assert(std::is_arithmetic<T>::value, "Expected a arithmetic type");

			if (lex.getToken() == Token::INT)
				num = lex.getIntegetValue();
			else if (lex.getToken() == Token::FLOAT)
				num = lex.getFloatValue();
			else
				lex.syntaxError("Expected number");

			lex.next();
		}

		explicit operator bool() const
		{ return static_cast<bool>(lex); }

	private:
		template<typename T>
		void parseArithmeticCell(T& num)
		{
			static_assert(std::is_arithmetic<T>::value, "Expected a arithmetic type");

			if (lex.getToken() == Token::L_BRACE)
			{
				lex.match(Token::L_BRACE);
				if (lex.getToken() != Token::R_BRACE)
				{
					parseExpression(num);
					lex.option(Token::COMMA);
				}
				else
					num = 0;
				lex.match(Token::R_BRACE);
			}
			else
				parseExpression(num);
		}

		void stringToString(std::string& string)
		{
			if (lex.getToken() == Token::STR)
				string = lex.getTokenText();
			lex.match(Token::STR);
		}

		template<typename T, size_t n>
		void stringToString(T(&string)[n])
		{
			const std::string& raw = lex.getTokenText();
			//while (!raw.back())
			//	raw.pop_back();

			if (raw.size() >= n)
				lex.syntaxError("initializer-string for current array is too long");

			for (size_t i = 0; i < raw.size(); ++i)
				string[i] = raw[i];
			string[raw.size()] = 0;

			lex.match(Token::STR);
		}

		void charatorStreamToString(std::string& string)
		{
			char temp;
			parseArithmeticCell(temp);
			string.clear();
			string.push_back(temp);
			while (lex.getToken() == Token::COMMA)
			{
				if (lex.next() == Token::R_BRACE)
					break;
				parseArithmeticCell(temp);
				string.push_back(temp);
			}
		}

		template<typename T, size_t n>
		void charatorStreamToString(T(&string)[n])
		{
			parseArithmeticCell(string[0]);
			for (size_t i = 1; i < n; i++)
			{
				lex.match(Token::COMMA);
				parseArithmeticCell(string[i]);
			}
			lex.option(Token::COMMA);
		}

		template<typename T>
		void parseString(T& string)
		{
			if (lex.getToken() == Token::L_BRACE)
			{
				lex.match(Token::L_BRACE);
				if (lex.getToken() == Token::STR)
				{
					stringToString(string);
					lex.option(Token::COMMA);
				}
				else if (lex.getToken() != Token::R_BRACE)
				{
					charatorStreamToString(string);
				}
				lex.match(Token::R_BRACE);
			}
			else if (lex.getToken() == Token::STR)
				stringToString(string);
			else
				lex.syntaxError("Expected string");
		}

		template<size_t n, typename T>
		void parseArray(T& array)
		{
			static_assert(n, "Array size shoudn't be zero!");

			lex.match(Token::L_BRACE);
			parserDispatcher(array[0]);
			for (size_t i = 1; i < n; i++)
			{
				lex.match(Token::COMMA);
				parserDispatcher(array[i]);
			}
			lex.option(Token::COMMA);
			lex.match(Token::R_BRACE);
		}

		template<typename CellT, typename T>
		void parseDynamicArray(T& dynamic_array)
		{
			lex.match(Token::L_BRACE);
			typename std::decay<CellT>::type temp;
			dynamic_array.clear();
			if (lex.getToken() != Token::R_BRACE)
			{
				parserDispatcher(temp);
				dynamic_array.push_back(std::move(temp));
				while (lex.getToken() == Token::COMMA)
				{
					if (lex.next() == Token::R_BRACE)
						break;
					parserDispatcher(temp);
					dynamic_array.push_back(std::move(temp));
				}
			}
			lex.match(Token::R_BRACE);
		}

		template<typename CellT, typename T>
		void parseSet(T& set)
		{
			lex.match(Token::L_BRACE);
			typename std::decay<CellT>::type temp;
			set.clear();
			if (lex.getToken() != Token::R_BRACE)
			{
				parserDispatcher(temp);
				set.emplace(std::move(temp));
				while (lex.getToken() == Token::COMMA)
				{
					if (lex.next() == Token::R_BRACE)
						break;
					parserDispatcher(temp);
					set.emplace(std::move(temp));
				}
			}
			lex.match(Token::R_BRACE);
		}

		template<typename CellT1, typename CellT2, typename T>
		void parseMap(T& map)
		{
			lex.match(Token::L_BRACE);
			std::pair<CellT1, CellT2> pair;
			map.clear();
			if (lex.getToken() == Token::L_BRACE)
			{
				parserDispatcher(pair);
				map.emplace(std::move(pair));
				while (lex.getToken() == Token::COMMA)
				{
					if (lex.next() != Token::L_BRACE)
						break;
					parserDispatcher(pair);
					map.emplace(std::move(pair));
				}
			}
			lex.match(Token::R_BRACE);
		}



		template<class T>
		typename std::enable_if<!std::is_arithmetic<T>::value>::type parserDispatcher(T&);

		template<typename T>
		typename std::enable_if<std::is_arithmetic<T>::value>::type parserDispatcher(T& num)
		{ parseArithmeticCell(num); }



		template<typename T, size_t n>
		void parserDispatcher(T(&array)[n])
		{ parseArray<n>(array); }

		template<typename T1, size_t n>
		void parserDispatcher(std::array<T1, n>& array)
		{ parseArray<n>(array); }



		template<size_t n>
		void parserDispatcher(char(&string)[n])
		{ parseString(string); }

		template<size_t n>
		void parserDispatcher(signed char(&string)[n])
		{ parseString(string); }

		template<size_t n>
		void parserDispatcher(unsigned char(&string)[n])
		{ parseString(string); }

		void parserDispatcher(std::string& string)
		{ parseString(string); }



		template<typename T, typename A>
		void parserDispatcher(std::vector<T, A>& vector)
		{ parseDynamicArray<T>(vector); }

		template<typename T, typename A>
		void parserDispatcher(std::deque<T, A>& vector)
		{ parseDynamicArray<T>(vector); }

		template<typename T, typename A>
		void parserDispatcher(std::list<T, A>& vector)
		{ parseDynamicArray<T>(vector); }



		template<typename T, typename C, typename A>
		void parserDispatcher(std::set<T, C, A>& set)
		{ parseSet<T>(set); }

		template<typename T, typename C, typename A>
		void parserDispatcher(std::multiset<T, C, A>& set)
		{ parseSet<T>(set); }

		template<typename T, typename H, typename C, typename A>
		void parserDispatcher(std::unordered_set<T, H, C, A>& set)
		{ parseSet<T>(set); }

		template<typename T, typename H, typename C, typename A>
		void parserDispatcher(std::unordered_multiset<T, H, C, A>& set)
		{ parseSet<T>(set); }



		template<typename T1, typename T2>
		void parserDispatcher(std::pair<T1, T2>& pair)
		{
			lex.match(Token::L_BRACE);
			typename std::decay<T1>::type a;
			parserDispatcher(a);
			lex.match(Token::COMMA);
			typename std::decay<T2>::type b;
			parserDispatcher(b);
			lex.option(Token::COMMA);
			lex.match(Token::R_BRACE);
			pair = std::make_pair<T1, T2>(std::move(a), std::move(b));
		}

		template<typename T1, typename T2, typename C, typename A>
		void parserDispatcher(std::map<T1, T2, C, A>& map)
		{ parseMap<T1, T2>(map); }

		template<typename T1, typename T2, typename C, typename A>
		void parserDispatcher(std::multimap<T1, T2, C, A>& map)
		{ parseMap<T1, T2>(map); }

		template<typename T1, typename T2, typename H, typename C, typename A>
		void parserDispatcher(std::unordered_map<T1, T2, H, C, A>& map)
		{ parseMap<T1, T2>(map); }

		template<typename T1, typename T2, typename H, typename C, typename A>
		void parserDispatcher(std::unordered_multimap<T1, T2, H, C, A>& map)
		{ parseMap<T1, T2>(map); }



		Lexer lex;
	};
} /* namespace: ezcfg */

#define EZCFG_REGISTER_STRUCT(T) template<> void ::ezcfg::Interpreter::parserDispatcher(T&)

#endif /* !__INTERPRETER_HPP__ */
