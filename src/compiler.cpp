#include <interpreter.hpp>

namespace ezcfg
{
	class Compiler
	{
	public:
		Compiler();

		void compile()
		{

		}

	private:
		Lexer lex;
		std::vector<std::string> current_name;
	};
}

int main(int argc, char** argv)
{

}
