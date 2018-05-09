// libnbt++ --- A C++ library for parsing, generating and manipulating NBT
// Copyright (C) 2018 Alexandre Szymocha <alexandre@szymocha.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef __GNUC__
# pragma once
#endif // !__GNUC__
#ifndef NBT_BYTECONVERTERS_H_
# define NBT_BYTECONVERTERS_H_

# include <cstdint>

namespace nbt::detail {

template <typename _Char>
static inline std::uint16_t
b2tos16(_Char const *buf)
{
	union
	{
		std::uint8_t byte[2];
		std::uint16_t val;
	} conv;
#if '\x11\x22\x33\x44' == 0x11'22'33'44
	conv.byte[0] = static_cast<std::uint8_t>(buf[1] & 0xFF);
	conv.byte[1] = static_cast<std::uint8_t>(buf[0] & 0xFF);
#else
	conv.byte[0] = static_cast<std::uint8_t>(buf[0] & 0xFF);
	conv.byte[1] = static_cast<std::uint8_t>(buf[1] & 0xFF);
#endif
	return conv.val;
}

template <typename _Char>
static inline std::uint32_t
b4tos32(_Char const *buf)
{
	union
	{
		std::uint8_t byte[4];
		std::uint32_t val;
	} conv;
#if '\x11\x22\x33\x44' == 0x11'22'33'44
	conv.byte[0] = static_cast<std::uint8_t>(buf[3] & 0xFF);
	conv.byte[1] = static_cast<std::uint8_t>(buf[2] & 0xFF);
	conv.byte[2] = static_cast<std::uint8_t>(buf[1] & 0xFF);
	conv.byte[3] = static_cast<std::uint8_t>(buf[0] & 0xFF);
#else
	conv.byte[0] = static_cast<std::uint8_t>(buf[0] & 0xFF);
	conv.byte[1] = static_cast<std::uint8_t>(buf[1] & 0xFF);
	conv.byte[2] = static_cast<std::uint8_t>(buf[2] & 0xFF);
	conv.byte[3] = static_cast<std::uint8_t>(buf[3] & 0xFF);
#endif
	return conv.val;
}

template <typename _Char>
static inline std::uint64_t
b8tos64(_Char const *buf)
{
	union
	{
		std::uint8_t byte[8];
		std::uint64_t val;
	} conv;
#if '\x11\x22\x33\x44' == 0x11'22'33'44
	conv.byte[0] = static_cast<std::uint8_t>(buf[7] & 0xFF);
	conv.byte[1] = static_cast<std::uint8_t>(buf[6] & 0xFF);
	conv.byte[2] = static_cast<std::uint8_t>(buf[5] & 0xFF);
	conv.byte[3] = static_cast<std::uint8_t>(buf[4] & 0xFF);
	conv.byte[4] = static_cast<std::uint8_t>(buf[3] & 0xFF);
	conv.byte[5] = static_cast<std::uint8_t>(buf[2] & 0xFF);
	conv.byte[6] = static_cast<std::uint8_t>(buf[1] & 0xFF);
	conv.byte[7] = static_cast<std::uint8_t>(buf[0] & 0xFF);
#else
	conv.byte[0] = static_cast<std::uint8_t>(buf[0] & 0xFF);
	conv.byte[1] = static_cast<std::uint8_t>(buf[1] & 0xFF);
	conv.byte[2] = static_cast<std::uint8_t>(buf[2] & 0xFF);
	conv.byte[3] = static_cast<std::uint8_t>(buf[3] & 0xFF);
	conv.byte[4] = static_cast<std::uint8_t>(buf[4] & 0xFF);
	conv.byte[5] = static_cast<std::uint8_t>(buf[5] & 0xFF);
	conv.byte[6] = static_cast<std::uint8_t>(buf[6] & 0xFF);
	conv.byte[7] = static_cast<std::uint8_t>(buf[7] & 0xFF);
#endif
	return conv.val;
}

template <typename _Char>
static inline float
b4toflt(_Char const *buf)
{
	union
	{
		std::uint8_t byte[4];
		float val;
	} conv;
#if '\x11\x22\x33\x44' == 0x11'22'33'44
	conv.byte[0] = static_cast<std::uint8_t>(buf[3] & 0xFF);
	conv.byte[1] = static_cast<std::uint8_t>(buf[2] & 0xFF);
	conv.byte[2] = static_cast<std::uint8_t>(buf[1] & 0xFF);
	conv.byte[3] = static_cast<std::uint8_t>(buf[0] & 0xFF);
#else
	conv.byte[0] = static_cast<std::uint8_t>(buf[0] & 0xFF);
	conv.byte[1] = static_cast<std::uint8_t>(buf[1] & 0xFF);
	conv.byte[2] = static_cast<std::uint8_t>(buf[2] & 0xFF);
	conv.byte[3] = static_cast<std::uint8_t>(buf[3] & 0xFF);
#endif
	return conv.val;
}

template <typename _Char>
static inline double
b8todbl(_Char const *buf)
{
	union
	{
		std::uint8_t byte[8];
		double val;
	} conv;
#if '\x11\x22\x33\x44' == 0x11'22'33'44
	conv.byte[0] = static_cast<std::uint8_t>(buf[7] & 0xFF);
	conv.byte[1] = static_cast<std::uint8_t>(buf[6] & 0xFF);
	conv.byte[2] = static_cast<std::uint8_t>(buf[5] & 0xFF);
	conv.byte[3] = static_cast<std::uint8_t>(buf[4] & 0xFF);
	conv.byte[4] = static_cast<std::uint8_t>(buf[3] & 0xFF);
	conv.byte[5] = static_cast<std::uint8_t>(buf[2] & 0xFF);
	conv.byte[6] = static_cast<std::uint8_t>(buf[1] & 0xFF);
	conv.byte[7] = static_cast<std::uint8_t>(buf[0] & 0xFF);
#else
	conv.byte[0] = static_cast<std::uint8_t>(buf[0] & 0xFF);
	conv.byte[1] = static_cast<std::uint8_t>(buf[1] & 0xFF);
	conv.byte[2] = static_cast<std::uint8_t>(buf[2] & 0xFF);
	conv.byte[3] = static_cast<std::uint8_t>(buf[3] & 0xFF);
	conv.byte[4] = static_cast<std::uint8_t>(buf[4] & 0xFF);
	conv.byte[5] = static_cast<std::uint8_t>(buf[5] & 0xFF);
	conv.byte[6] = static_cast<std::uint8_t>(buf[6] & 0xFF);
	conv.byte[7] = static_cast<std::uint8_t>(buf[7] & 0xFF);
#endif
	return conv.val;
}

} // nbt::detail

#endif // !NBT_BYTECONVERTERS_H_
