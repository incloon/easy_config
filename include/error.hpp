/********************************************************************\
|*  This file is part of easy_config.                               *|
|*                                                                  *|
|*  Copyright (c) 2021-2022 Incloon                                 *|
|*                                                                  *|
|* easy_config is free software : you can redistribute it and/or    *|
|* modify it under the terms of the GNU Lesser General Public       *|
|* License as published by the Free Software Foundation, either     *|
|* version 3 of the License, or (at your option) any later version. *|
|*                                                                  *|
|* easy_config is distributed in the hope that it will be useful,   *|
|* but WITHOUT ANY WARRANTY; without even the implied warranty of   *|
|* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the      *|
|* GNU Lesser General Public License for more details.              *|
|*                                                                  *|
|* You should have received a copy of the GNU Lesser General Public *|
|* License along with easy_config.                                  *|
|* If not, see < https://www.gnu.org/licenses/>.                    *|
\********************************************************************/
#pragma once

#include <stdexcept>
#include <string>

namespace ezcfg
{
	class ParseError : public std::runtime_error
	{
	public:
		ParseError(std::string file, size_t line, size_t column, std::string message)
			: std::runtime_error("")
			, file_name{ std::move(file) }
			, line_num{ line }
			, col_num{ column }
			, msg{ std::move(message) }
		{
			formatted = file_name + ":" + std::to_string(line_num) + ":" + std::to_string(col_num) + ": error: " + msg;
			static_cast<std::runtime_error&>(*this) = std::runtime_error(formatted);
		}

		const std::string& file() const noexcept { return file_name; }
		size_t line() const noexcept { return line_num; }
		size_t column() const noexcept { return col_num; }
		const std::string& message() const noexcept { return msg; }

	private:
		std::string file_name;
		size_t line_num;
		size_t col_num;
		std::string msg;
		std::string formatted;
	};
} /* namespace: ezcfg */
