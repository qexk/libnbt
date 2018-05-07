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
#ifndef NBT_PARSER_H_
# define NBT_PARSER_H_

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
,	S1, S2, S3, S4, S5, S6, S7, S8, S9, SA, SB, SC
,	S7A, S8A, S9A, SBA, SCA
,	S9B, SAB
,	NTS, NT
,	F
};

template <typename _In_char>
static inline state
state_of_tag(_In_char const tag)
{
	switch (static_cast<unsigned>(tag))
	{
	case 0x01U: return state::S1;
	case 0x02U: return state::S2;
	case 0x03U: return state::S3;
	case 0x04U: return state::S4;
	case 0x05U: return state::S5;
	case 0x06U: return state::S6;
	case 0x07U: return state::S7;
	case 0x08U: return state::S8;
	case 0x09U: return state::S9;
	case 0x0AU: return state::SA;
	case 0x0BU: return state::SB;
	case 0x0CU: return state::SC;
	default:   return state::F;
	}
}

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

# define NBT_HAS__BUILDER_(method)                              \
template <typename T>                                           \
static inline constexpr auto                                    \
_Has_##method(int)                                              \
->	decltype(std::declval<T>().method(0), std::true_type{}) \
{                                                               \
	return {};                                              \
}                                                               \
                                                                \
template <typename T>                                           \
static inline constexpr auto                                    \
_Has_##method(...)                                              \
->	std::false_type                                         \
{                                                               \
	return {};                                              \
}                                                               \
                                                                \
template <typename T>                                           \
static inline constexpr auto has_##method = _Has_##method<T>({})

NBT_HAS__BUILDER_( reserve );
NBT_HAS__BUILDER_( try_emplace );

# undef NBT_HAS__BUILDER_
} // detail

template
<	template <class> typename _Allocator = std::allocator
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
,	typename _In_char = char
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
	static constexpr typename _In_traits::int_type tag_nul{0x00};
	static constexpr typename _In_traits::int_type tag_byt{0x01};
	static constexpr typename _In_traits::int_type tag_sht{0x02};
	static constexpr typename _In_traits::int_type tag_int{0x03};
	static constexpr typename _In_traits::int_type tag_lng{0x04};
	static constexpr typename _In_traits::int_type tag_flt{0x05};
	static constexpr typename _In_traits::int_type tag_dbl{0x06};
	static constexpr typename _In_traits::int_type tag_bya{0x07};
	static constexpr typename _In_traits::int_type tag_str{0x08};
	static constexpr typename _In_traits::int_type tag_lst{0x09};
	static constexpr typename _In_traits::int_type tag_cpd{0x0A};
	static constexpr typename _In_traits::int_type tag_ina{0x0B};
	static constexpr typename _In_traits::int_type tag_lna{0x0C};
	using detail::state;
	std::unordered_map
	<	state
	,	detail::default_map<typename _In_traits::int_type, int>
	> const trans =
	{	{ state::F, {{ _In_traits::eof(), '0' }} }
	,	{	state::S
		,	{	{ tag_byt,  '1' }
			,	{ tag_sht,  '2' }
			,	{ tag_int,  '3' }
			,	{ tag_lng,  '4' }
			,	{ tag_flt,  '5' }
			,	{ tag_dbl,  '6' }
			,	{ tag_bya,  '7' }
			,	{ tag_str,  '8' }
			,	{ tag_lst,  '9' }
			,	{ tag_cpd,  'A' }
			,	{ tag_ina,  'B' }
			,	{ tag_lna,  'C' }
			}
		}
	,	{	state::NTS
		,	{	{ tag_nul,  'AE' }
			,	{ detail::_, 'AB' }
			}
		}
	,	{ state::S1,  {{ detail::_, '1A' }} }
	,	{ state::S2,  {{ detail::_, '2A' }} }
	,	{ state::S3,  {{ detail::_, '3A' }} }
	,	{ state::S4,  {{ detail::_, '4A' }} }
	,	{ state::S5,  {{ detail::_, '5A' }} }
	,	{ state::S6,  {{ detail::_, '6A' }} }
	,	{ state::S7,  {{ detail::_, '7A' }} }
	,	{ state::S7A, {{ detail::_, '7B' }} }
	,	{ state::S8,  {{ detail::_, '8A' }} }
	,	{ state::S8A, {{ detail::_, '8B' }} }
	,	{ state::S9,  {{ detail::_, '9A' }} }
	,	{ state::S9A, {{ detail::_, '9B' }} }
	,	{ state::S9B, {{ detail::_, '9C' }} }
	,	{ state::SA,  {{ detail::_, 'AA' }} }
	,	{ state::NT,  {{ detail::_, 'NT' }} }
	,	{ state::SAB, {{ detail::_, 'AC' }} }
	,	{ state::SB,  {{ detail::_, 'BA' }} }
	,	{ state::SBA, {{ detail::_, 'BB' }} }
	,	{ state::SC,  {{ detail::_, 'CA' }} }
	,	{ state::SCA, {{ detail::_, 'CB' }} }
	};

	auto deleter = [] (void *ptr_raw) {
		static auto __a = _Allocator<_Node>();
		auto ptr = reinterpret_cast<_Node *>(ptr_raw);
		_A::destroy(__a, ptr);
		_A::deallocate(__a, ptr, 1);
	};

	std::deque<_Node_ptr> ret;
	std::stack<state> ss;
	ss.push(state::F);
	ss.push(state::S);

loop:
	switch (trans.at(ss.top()).at(in.peek()))
	{
	case '1':
		in.get();
		ss.pop();
		ss.push(state::S1);
		goto loop;
	case '1A':
		{
			_Node *node = _A::allocate(__a, 1);
			_A::construct
			(	__a, node
			,	static_cast<_Byte>(in.get())
			);
			ret.push_front(_Node_ptr(node));
			ss.pop();
			goto loop;
		}
	case '2':
		in.get();
		ss.pop();
		ss.push(state::S2);
		goto loop;
	case '2A':
		{
			_In_char buf[2];
			in.read(buf, sizeof(buf));
			_Node *node = _A::allocate(__a, 1);
			_A::construct
			(	__a, node
			,	static_cast<_Short>(detail::b2tos16(buf))
			);
			ret.push_front(_Node_ptr(node));
			ss.pop();
			goto loop;
		}
	case '3':
		in.get();
		ss.pop();
		ss.push(state::S3);
		goto loop;
	case '3A':
		{
			_In_char buf[4];
			in.read(buf, sizeof(buf));
			_Node *node = _A::allocate(__a, 1);
			_A::construct
			(	__a, node
			,	static_cast<_Int>(detail::b4tos32(buf))
			);
			ret.push_front(_Node_ptr(node));
			ss.pop();
			goto loop;
		}
	case '4':
		in.get();
		ss.pop();
		ss.push(state::S4);
		goto loop;
	case '4A':
		{
			_In_char buf[8];
			in.read(buf, sizeof(buf));
			_Node *node = _A::allocate(__a, 1);
			_A::construct
			(	__a, node
			,	static_cast<_Long>(detail::b8tos64(buf))
			);
			ret.push_front(_Node_ptr(node));
			ss.pop();
			goto loop;
		}
	case '5':
		in.get();
		ss.pop();
		ss.push(state::S5);
		goto loop;
	case '5A':
		{
			_In_char buf[4];
			in.read(buf, sizeof(buf));
			auto repr = detail::b4tos32(buf);
			_Node *node = _A::allocate(__a, 1);
			_A::construct
			(	__a, node
			,	static_cast<_Float>
				(	*reinterpret_cast<float *>(&repr)
				)
			);
			ret.push_front(_Node_ptr(node));
			ss.pop();
			goto loop;
		}
	case '6':
		in.get();
		ss.pop();
		ss.push(state::S6);
		goto loop;
	case '6A':
		{
			_In_char buf[8];
			in.read(buf, sizeof(buf));
			auto repr = detail::b8tos64(buf);
			_Node *node = _A::allocate(__a, 1);
			_A::construct
			(	__a, node
			,	static_cast<_Double>
				(	*reinterpret_cast<double *>(&repr)
				)
			);
			ret.push_front(_Node_ptr(node));
			ss.pop();
			goto loop;
		}
	case '7':
		in.get();
		ss.pop();
		ss.push(state::S7);
		goto loop;
	case '7A':
		ss.pop();
		ss.push(state::S7A);
		ss.push(state::S3);
		goto loop;
	case '7B':
		{
			auto const &len = std::get<2>(*ret.front());
			_Byte_array_type cont;
			if (len > 0)
			{
				auto const buf =
					std::make_unique<_In_char[]>(len);
				in.read(buf.get(), len);
				if constexpr
				(	detail::has_reserve
					<	_Byte_array_type
					>
				)
					cont.reserve(len);
				std::copy
				(	buf.get(), buf.get() + len
				,	std::back_inserter(cont)
				);
			}
			_Node * const node = _A::allocate(__a, 1);
			_A::construct
			(	__a, node
			,	std::move(cont)
			);
			ret.pop_front();
			ret.push_front(_Node_ptr(node));
			ss.pop();
			goto loop;
		}
	case '8':
		in.get();
		ss.pop();
		ss.push(state::S8);
		goto loop;
	case '8A':
		ss.pop();
		ss.push(state::S8A);
		ss.push(state::S2);
		goto loop;
	case '8B':
		{
			std::size_t const len = std::get<1>(*ret.front());
			auto const buf = std::make_unique<_In_char[]>(len);
			in.read(buf.get(), len);
			_String_type cont;
			if constexpr(detail::has_reserve<_String_type>)
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
			ret.pop_front();
			ret.push_front(_Node_ptr(node));
			ss.pop();
			goto loop;
		}
	case '9':
		in.get();
		ss.pop();
		ss.push(state::S9);
		goto loop;
	case '9A':
		{
			_Node * const node = _A::allocate(__a, 1);
			_A::construct
			(	__a, node
			,	_List_type()
			);
			ret.push_front(_Node_ptr(node));
			ss.pop();
			ss.push(state::S9A);
			ss.push(state::S3);
			ss.push(state::S1);
			goto loop;
		}
	case '9B':
		{
			auto const &count = std::get<2>(*ret.front());
			auto const &tag = std::get<0>(*ret[1]);
			if constexpr (detail::has_reserve<_List_type>)
				std::get<8>(*ret[2]).reserve(count);
			ss.pop();
			if (count > 0 && tag != tag_nul)
			{
				ss.push(state::S9B);
				ss.push(detail::state_of_tag(tag));
			}
			else
			{
				ret.pop_front();
				ret.pop_front();
			}
			goto loop;
		}
	case '9C':
		{
			auto &count = std::get<2>(*ret[1]);
			auto const &tag = std::get<0>(*ret[2]);
			std::get<8>(*ret[3]).emplace_back
			(	reinterpret_cast<void *>
				(	ret.front().release()
				)
			,	deleter
			);
			ret.pop_front();
			ss.pop();
			if (--count > 0)
			{
				ss.push(state::S9B);
				ss.push(detail::state_of_tag(tag));
			}
			else
			{
				ret.pop_front();
				ret.pop_front();
			}
			goto loop;
		}
	case 'A':
		in.get();
		ss.pop();
		ss.push(state::SA);
		goto loop;
	case 'AA':
		{
			_Node * const node = _A::allocate(__a, 1);
			_A::construct
			(	__a, node
			,	_Compound_type()
			);
			ret.push_front(_Node_ptr(node));
			ss.pop();
			ss.push(state::NTS);
			goto loop;
		}
	case 'AB':
		ss.pop();
		ss.push(state::NT);
		ss.push(state::S8);
		ss.push(state::S1);
		goto loop;
	case 'NT':
		{
			auto const &tag = std::get<0>(*ret[1]);
			ss.pop();
			ss.push(state::SAB);
			ss.push(detail::state_of_tag(tag));
			goto loop;
		}
	case 'AC':
		{
			if constexpr
			(	detail::has_try_emplace<_Compound_type>
			)
			{
				std::get<9>(*ret[3]).try_emplace
				(	std::get<7>(*ret[1])
				,	reinterpret_cast<void *>
					(	ret.front().release()
					)
				,	deleter
				);
			}
			else
			{
				typename _Compound_type::mapped_type value
				(	reinterpret_cast<void *>
					(	ret.front().release()
					)
				,	deleter
				);
				std::get<9>(*ret[3]).emplace
				(	std::get<7>(*ret[1])
				,	std::move(value)
				);
			}
			ret.pop_front();
			ret.pop_front();
			ret.pop_front();
			ss.pop();
			ss.push(state::NTS);
			goto loop;
		}
	case 'AE':
		in.get();
		ss.pop();
		goto loop;
	case 'B':
		in.get();
		ss.pop();
		ss.push(state::SB);
		goto loop;
	case 'BA':
		ss.pop();
		ss.push(state::SBA);
		ss.push(state::S3);
		goto loop;
	case 'BB':
		{
			auto const count = std::get<2>(*ret.front());
			_Int_array_type cont;
			if (count > 0)
			{
				auto const len = count * 4;
				auto const buf =
					std::make_unique<_In_char[]>(len);
				in.read(buf.get(), len);
				if constexpr
				(	detail::has_reserve
					<	_Int_array_type
					>
				)
					cont.reserve(count);
				auto inserter = std::back_inserter(cont);
				for (auto i = 0; i < count; ++i)
					inserter = static_cast<_Int>
					(	detail::b4tos32
						(	buf.get() + i * 4
						)
					);
			}
			_Node * const node = _A::allocate(__a, 1);
			_A::construct
			(	__a, node
			,	std::move(cont)
			);
			ret.pop_front();
			ret.push_front(_Node_ptr(node));
			ss.pop();
			goto loop;
		}
	case 'C':
		in.get();
		ss.pop();
		ss.push(state::SC);
		goto loop;
	case 'CA':
		ss.pop();
		ss.push(state::SCA);
		ss.push(state::S3);
		goto loop;
	case 'CB':
		{
			auto const count = std::get<2>(*ret.front());
			_Long_array_type cont;
			if (count > 0)
			{
				auto const len = count * 8;
				auto const buf =
					std::make_unique<_In_char[]>(len);
				in.read(buf.get(), len);
				if constexpr
				(	detail::has_reserve
					<	_Long_array_type
					>
				)
					cont.reserve(len);
				auto inserter = std::back_inserter(cont);
				for (auto i = 0; i < count; ++i)
					inserter = static_cast<_Long>
					(	detail::b8tos64
						(	buf.get() + i * 8
						)
					);
			}
			_Node * const node = _A::allocate(__a, 1);
			_A::construct
			(	__a, node
			,	std::move(cont)
			);
			ret.pop_front();
			ret.push_front(_Node_ptr(node));
			ss.pop();
			goto loop;
		}
	case '0':
		goto end;
	}
end:
	return std::move(ret.front());
}

} // nbt

#endif // !NBT_PARSER_H_
