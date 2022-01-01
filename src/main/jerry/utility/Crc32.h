/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2022 Sven Lukas
 *
 * Jerry is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Jerry is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with Jerry.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef JERRY_UTILITY_CRC32_H_
#define JERRY_UTILITY_CRC32_H_

#include <cstdint>
#include <string>


namespace jerry {
namespace utility {

class CRC32 {
public:
	CRC32(std::uint32_t initialValue = 0);

	void pushData(const void* data, std::size_t length);

	std::uint32_t get() const noexcept;

private:
	std::uint32_t value;
};
std::uint32_t getCRC32(const void* data, std::size_t length, std::uint32_t crc32 = 0);

} /* namespace utility */
} /* namespace jerry */

#endif /* JERRY_UTILITY_CRC32_H_ */
