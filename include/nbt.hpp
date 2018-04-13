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
#ifndef NBT_H_
# define NBT_H_

# include <istream>
# include <memory>
# include <limits>
# include <stack>
# include <string>
# include <type_traits>
# include <unordered_map>
# include <variant>
# include <vector>
# include <cstring>

# include <boost/iostreams/stream.hpp>
# include <boost/iostreams/device/array.hpp>

namespace nbt {

namespace detail {

template <class _List>
struct integer_list_hash
{
	std::size_t operator()(_List const &l) const noexcept
	{
		std::size_t seed = std::distance(l.cbegin(), l.cend());
		for (auto const &i : l) {
			seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};

} // detail

template
<	typename _In_char = char
,	template <class> typename _Allocator = std::allocator
,	typename _Char = char
,	typename _Byte = std::int_least8_t
,	typename _Short = std::int_least16_t
,	typename _Int = std::int_least32_t
,	typename _Long = std::int_least64_t
,	typename _Float = float
,	typename _Double = double
,	template <class, class> typename _Byte_array = std::vector
,	template <class, class, class> typename _String = std::basic_string
,	template <class, class> typename _List = std::vector
,	template <class, class, class, class, class>
		typename _Compound = std::unordered_map
,	template <class, class> typename _Int_array = std::vector
,	template <class, class> typename _Long_array = std::vector
,	typename _Byte_array_type = _Byte_array<_Byte, _Allocator<_Byte>>
,	typename _String_type = _String
	<	char
	,	std::char_traits<char>
	,	_Allocator<char>
	>
,	typename _List_type = _List
	<	std::unique_ptr<void, void (*) (void const *)>
	,	_Allocator<std::unique_ptr<void, void (*) (void const *)>>
	>
,	typename _Compound_type = _Compound
	<	_String_type
	,	std::unique_ptr<void, void (*) (void const *)>
	,	detail::integer_list_hash<_String_type>
	,	std::equal_to<_String_type>
	,	_Allocator<std::unique_ptr<void, void (*) (void const *)>>
	>
,	typename _Int_array_type = _Int_array<_Int, _Allocator<_Int>>
,	typename _Long_array_type = _Long_array<_Long, _Allocator<_Long>>
,	typename _Node = std::variant
	<	_Char, _Byte, _Short, _Int, _Long, _Float, _Double
	,	_Byte_array_type, _String_type, _List_type, _Compound_type
	,	_Int_array_type, _Long_array_type
	>
>
std::unique_ptr<_Node>
parse
(	std::basic_istream<_In_char> &&in
//,	_Allocator const & __a = _Allocator()
)
{
	(void)(in);
	return std::unique_ptr<_Node>();
}


} // nbt

#endif // !NBT_H_
