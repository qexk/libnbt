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

# include <algorithm>
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
	std::size_t
	operator()(_List const &l) const noexcept
	{
		std::size_t _Seed = std::distance(l.cbegin(), l.cend());
		for (auto const &_I : l)
			_Seed ^= _I + 0x9e3779b9 +
				(_Seed << 6) + (_Seed >> 2);
		return _Seed;
	}
};

template <typename _Char>
static inline std::uint16_t
b2tos16(_Char const *buf)
{
	return
#if '\x11\x22\x33\x44' == 0x11'22'33'44
		(static_cast<std::uint32_t>(buf[0]) << 010 & 0xFF00) |
		(static_cast<std::uint32_t>(buf[1]) & 0x00FF);
#else
		(static_cast<std::uint32_t>(buf[1]) << 010 & 0xFF00) |
		(static_cast<std::uint32_t>(buf[0]) & 0x00FF);
#endif
}

template <typename _Char>
static inline std::uint32_t
b4tos32(_Char const *buf)
{
	return
#if '\x11\x22\x33\x44' == 0x11'22'33'44
		(static_cast<std::uint32_t>(buf[0]) << 030 & 0xFF000000) |
		(static_cast<std::uint32_t>(buf[1]) << 020 & 0x00FF0000) |
		(static_cast<std::uint32_t>(buf[2]) << 010 & 0x0000FF00) |
		(static_cast<std::uint32_t>(buf[3]) & 0x000000FF);
#else
		(static_cast<std::uint32_t>(buf[3]) << 030 & 0xFF000000) |
		(static_cast<std::uint32_t>(buf[2]) << 020 & 0x00FF0000) |
		(static_cast<std::uint32_t>(buf[1]) << 010 & 0x0000FF00) |
		(static_cast<std::uint32_t>(buf[0]) & 0x000000FF);
#endif
}

template <typename _Char>
static inline std::uint64_t
b8tos64(_Char const *buf)
{
	return
#if '\x11\x22\x33\x44' == 0x11'22'33'44
		(static_cast<std::uint64_t>(buf[0]) << 070 & 0xFFL << 070) |
		(static_cast<std::uint64_t>(buf[1]) << 060 & 0xFFL << 060) |
		(static_cast<std::uint64_t>(buf[2]) << 050 & 0xFFL << 050) |
		(static_cast<std::uint64_t>(buf[3]) << 040 & 0xFFL << 040) |
		(static_cast<std::uint64_t>(buf[4]) << 030 & 0xFFL << 030) |
		(static_cast<std::uint64_t>(buf[5]) << 020 & 0xFFL << 020) |
		(static_cast<std::uint64_t>(buf[6]) << 010 & 0xFFL << 010) |
		(static_cast<std::uint64_t>(buf[7]) & 0xFFL);
#else
		(static_cast<std::uint64_t>(buf[7]) << 070 & 0xFFL << 070) |
		(static_cast<std::uint64_t>(buf[6]) << 060 & 0xFFL << 060) |
		(static_cast<std::uint64_t>(buf[5]) << 050 & 0xFFL << 050) |
		(static_cast<std::uint64_t>(buf[4]) << 040 & 0xFFL << 040) |
		(static_cast<std::uint64_t>(buf[3]) << 030 & 0xFFL << 030) |
		(static_cast<std::uint64_t>(buf[2]) << 020 & 0xFFL << 020) |
		(static_cast<std::uint64_t>(buf[1]) << 010 & 0xFFL << 010) |
		(static_cast<std::uint64_t>(buf[0]) & 0xFFL);
#endif
}

enum class state
{	S
,	S1, S2, S3, S4, S5, S6, S7, S8, S9, S10, S11, S12
,	F
};

template
<	class _Key_real
,	class _Value
,	class _Key = std::variant<std::monostate, _Key_real>
,	class _Hash = std::hash<_Key>
,	class _Equal = std::equal_to<_Key>
,	class _Allocator = std::allocator<std::pair<_Key const, _Value>>
,	class _Base = std::unordered_map
	<	_Key
	,	_Value
	,	_Hash
	,	_Equal
	,	_Allocator
	>
>
struct default_map : public _Base
{
	using _Base::unordered_map;

	virtual _Value &
	at(_Key_real const &key)
	{
		if (auto it = _Base::find(key); it != _Base::end())
			return it->second;
		else
			return _Base::at(std::monostate());
	}

	virtual _Value const &
	at(_Key_real const &key) const
	{
		if (auto it = _Base::find(key); it != _Base::cend())
			return it->second;
		else
			return _Base::at(std::monostate());
	}

	virtual
	~default_map()
	{}
};

static constexpr std::monostate _;

template <typename T>
static inline constexpr auto
_Has_reserve(int)
->	decltype(std::declval<T>().reserve(0), std::true_type{})
{
	return {};
}

template <typename T>
static inline constexpr auto
_Has_reserve(...)
->	std::false_type
{
	return {};
}

template <typename T>
static inline constexpr auto has_reserve = _Has_reserve<T>({});

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
	<	std::unique_ptr<void, void (*) (void *)>
	,	_Allocator<std::unique_ptr<void, void (*) (void *)>>
	>
,	typename _Compound_type = _Compound
	<	_String_type
	,	std::unique_ptr<void, void (*) (void *)>
	,	detail::integer_list_hash<_String_type>
	,	std::equal_to<_String_type>
	,	_Allocator<std::unique_ptr<void, void (*) (void *)>>
	>
,	typename _Int_array_type = _Int_array<_Int, _Allocator<_Int>>
,	typename _Long_array_type = _Long_array<_Long, _Allocator<_Long>>
,	typename _Node = std::variant
	<	_Byte, _Short, _Int, _Long, _Float, _Double
	,	_Byte_array_type, _String_type, _List_type, _Compound_type
	,	_Int_array_type, _Long_array_type
	>
>
std::unique_ptr<_Node>
parse
(	std::basic_istream<_In_char> &in
,	_Allocator<_Node> __a = _Allocator<_Node>()
)
{
	using _Node_ptr = std::unique_ptr<_Node>;
	using _A = typename std::allocator_traits<_Allocator<_Node>>;
	using _In_traits = typename std::decay_t<decltype(in)>::traits_type;
	// static constexpr typename _In_traits::int_type _Tag_nul{0x00};
	static constexpr typename _In_traits::int_type _Tag_byt{0x01};
	static constexpr typename _In_traits::int_type _Tag_sht{0x02};
	static constexpr typename _In_traits::int_type _Tag_int{0x03};
	static constexpr typename _In_traits::int_type _Tag_lng{0x04};
	static constexpr typename _In_traits::int_type _Tag_flt{0x05};
	static constexpr typename _In_traits::int_type _Tag_dbl{0x06};
	static constexpr typename _In_traits::int_type _Tag_bya{0x07};
	// static constexpr typename _In_traits::int_type _Tag_str{0x08};
	// static constexpr typename _In_traits::int_type _Tag_lst{0x09};
	// static constexpr typename _In_traits::int_type _Tag_cpd{0x0A};
	// static constexpr typename _In_traits::int_type _Tag_ina{0x0B};
	// static constexpr typename _In_traits::int_type _Tag_lna{0x0C};
	using detail::state;
	std::unordered_map
	<	state
	,	detail::default_map<typename _In_traits::int_type, int>
	> const trans =
	{	{ state::F, {{ _In_traits::eof(), '0' }} }
	,	{	state::S
		,	{	{ _Tag_byt,  '1' }
			,	{ _Tag_sht,  '2' }
			,	{ _Tag_int,  '3' }
			,	{ _Tag_lng,  '4' }
			,	{ _Tag_flt,  '5' }
			,	{ _Tag_dbl,  '6' }
			,	{ _Tag_bya,  '7' }
			}
		}
	,	{ state::S2, {{ detail::_, '2A' }} }
	,	{ state::S3, {{ detail::_, '3A' }} }
	,	{ state::S7, {{ detail::_, '7A' }} }
	};

	std::stack<_Node_ptr> ret;
	std::stack<state> ss;
	ss.push(state::F);
	ss.push(state::S);

loop:
	switch (trans.at(ss.top()).at(in.peek()))
	{
	case '1':
		{
			ss.pop();
			in.get();
			_Node *node = _A::allocate(__a, 1);
			_A::construct
			(	__a, node
			,	static_cast<_Byte>(in.get())
			);
			ret.push(_Node_ptr(node));
			goto loop;
		}
	case '2':
		ss.pop();
		in.get();
		ss.push(state::S2);
		goto loop;
	case '2A':
		{
			ss.pop();
			_Node *node = _A::allocate(__a, 1);
			_In_char buf[2];
			in.read(buf, sizeof(buf));
			_A::construct
			(	__a, node
			,	static_cast<_Short>(detail::b2tos16(buf))
			);
			ret.push(_Node_ptr(node));
			goto loop;
		}
	case '3':
		ss.pop();
		in.get();
		ss.push(state::S3);
		goto loop;
	case '3A':
		{
			ss.pop();
			_Node *node = _A::allocate(__a, 1);
			_In_char buf[4];
			in.read(buf, sizeof(buf));
			_A::construct
			(	__a, node
			,	static_cast<_Int>(detail::b4tos32(buf))
			);
			ret.push(_Node_ptr(node));
			goto loop;
		}
	case '4':
		{
			ss.pop();
			in.get();
			_Node *node = _A::allocate(__a, 1);
			_In_char buf[8];
			in.read(buf, sizeof(buf));
			_A::construct
			(	__a, node
			,	static_cast<_Long>(detail::b8tos64(buf))
			);
			ret.push(_Node_ptr(node));
			goto loop;
		}
	case '5':
		{
			ss.pop();
			in.get();
			_Node *node = _A::allocate(__a, 1);
			_In_char buf[4];
			in.read(buf, sizeof(buf));
			auto repr = detail::b4tos32(buf);
			_A::construct
			(	__a, node
			,	static_cast<_Float>
				(	*reinterpret_cast<float *>(&repr)
				)
			);
			ret.push(_Node_ptr(node));
			goto loop;
		}
	case '6':
		{
			ss.pop();
			in.get();
			_Node *node = _A::allocate(__a, 1);
			_In_char buf[8];
			in.read(buf, sizeof(buf));
			auto repr = detail::b8tos64(buf);
			_A::construct
			(	__a, node
			,	static_cast<_Double>
				(	*reinterpret_cast<double *>(&repr)
				)
			);
			ret.push(_Node_ptr(node));
			goto loop;
		}
	case '7':
		ss.pop();
		in.get();
		ss.push(state::S7);
		ss.push(state::S3);
		goto loop;
	case '7A':
		{
			ss.pop();
			auto const &len = std::get<2>(*ret.top());
			auto const buf = std::make_unique<_In_char[]>(len);
			in.read(buf.get(), len);
			_Byte_array_type cont;
			if constexpr(detail::has_reserve<_Byte_array_type>)
				cont.reserve(len);
			std::copy
			(	buf.get(), buf.get() + len
			,	std::back_inserter(cont)
			);
			_Node * const node = _A::allocate(__a, 1);
			_A::construct
			(	__a, node
			,	std::move(cont)
			);
			ret.pop();
			ret.push(_Node_ptr(node));
			goto loop;
		}
	case '0':
		goto end;
	}
end:
	return std::move(ret.top());
}

} // nbt

#endif // !NBT_H_
