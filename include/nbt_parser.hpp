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
# include <fstream>
# include <istream>
# include <memory>
# include <limits>
# include <sstream>
# include <stack>
# include <string>
# include <type_traits>
# include <unordered_map>
# include <variant>
# include <vector>
# include <cstring>

# ifndef NBT_NO_BOOST
#  include <boost/iostreams/traits.hpp>
#  include <boost/iostreams/filtering_stream.hpp>
#  include <boost/iostreams/filter/gzip.hpp>
#  include <boost/iostreams/filter/zlib.hpp>
# endif // !NBT_NO_BOOST

namespace nbt {

namespace detail {

template <class _List>
struct integer_list_hash
{
	std::size_t
	operator()(_List const &l) const noexcept
	{
		std::size_t seed = std::distance(l.cbegin(), l.cend());
		for (auto const &_I : l)
			seed ^= _I + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		return seed;
	}
};

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

enum class parsing
{	implicit_compound
,	no_implicit
};

# define NBT_PARSER_TEMPLATE_DECLARATION                                   \
template                                                                   \
<	parsing _Policy = parsing::implicit_compound                       \
,	template <class> typename _Allocator = std::allocator              \
,	typename _Char = char                                              \
,	typename _Byte = std::int_least8_t                                 \
,	typename _Short = std::int_least16_t                               \
,	typename _Int = std::int_least32_t                                 \
,	typename _Long = std::int_least64_t                                \
,	typename _Float = float                                            \
,	typename _Double = double                                          \
,	template <class, class> typename _Byte_array = std::vector         \
,	template <class, class, class> typename _String = std::basic_string\
,	template <class, class> typename _List = std::vector               \
,	template <class, class, class, class, class>                       \
		typename _Compound = std::unordered_map                    \
,	template <class, class> typename _Int_array = std::vector          \
,	template <class, class> typename _Long_array = std::vector         \
,	typename _Byte_array_type = _Byte_array<_Byte, _Allocator<_Byte>>  \
,	typename _String_type = _String                                    \
	<	char                                                       \
	,	std::char_traits<char>                                     \
	,	_Allocator<char>                                           \
	>                                                                  \
,	typename _List_type = _List                                        \
	<	std::unique_ptr<void, void (*) (void *)>                   \
	,	_Allocator<std::unique_ptr<void, void (*) (void *)>>       \
	>                                                                  \
,	typename _Compound_type = _Compound                                \
	<	_String_type                                               \
	,	std::unique_ptr<void, void (*) (void *)>                   \
	,	detail::integer_list_hash<_String_type>                    \
	,	std::equal_to<_String_type>                                \
	,	_Allocator<std::unique_ptr<void, void (*) (void *)>>       \
	>                                                                  \
,	typename _Int_array_type = _Int_array<_Int, _Allocator<_Int>>      \
,	typename _Long_array_type = _Long_array<_Long, _Allocator<_Long>>  \
,	typename _Node = std::variant                                      \
	<	_Byte, _Short, _Int, _Long, _Float, _Double                \
	,	_Byte_array_type, _String_type, _List_type, _Compound_type \
	,	_Int_array_type, _Long_array_type                          \
	>                                                                  \
,	typename _In_char = char                                           \
>

NBT_PARSER_TEMPLATE_DECLARATION
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
	using _Byte_to_case = detail::default_map
	<	typename _In_traits::int_type
	,	int
	>;
	std::unordered_map<state, _Byte_to_case> const trans
	 = {	{ state::F, {{ _In_traits::eof(), '0' }} }
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
		,	_Policy == parsing::implicit_compound
			?	_Byte_to_case
				{	{ tag_nul,  'AE' }
				,	{ _In_traits::eof(), '0' }
				,	{ detail::_, 'AB' }
				}
			:	_Byte_to_case
				{	{ tag_nul,  'AE' }
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
	if constexpr (_Policy == parsing::implicit_compound)
	{
		ss.push(state::SA);
	}
	else
	{
		ss.push(state::F);
		ss.push(state::S);
	}

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
			_Node *node = _A::allocate(__a, 1);
			_A::construct
			(	__a, node
			,	static_cast<_Float>(detail::b4toflt(buf))
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
			_Node *node = _A::allocate(__a, 1);
			_A::construct
			(	__a, node
			,	static_cast<_Double>(detail::b8todbl(buf))
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
			_Byte_array_type cont;
			auto const len = *std::get_if<2>(ret.front().get());
			if (len > 0)
			{
				auto const buf
					 = std::make_unique<_In_char[]>
					 (	len
					 );
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
			std::size_t const len
				 = *std::get_if<1>(ret.front().get());
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
			auto const count
				 = *std::get_if<2>(ret.front().get());
			auto const tag
				 = *std::get_if<0>(ret[1].get());
			if constexpr (detail::has_reserve<_List_type>)
				std::get_if<8>(ret[2].get())
					->reserve(count);
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
			auto const tag = *std::get_if<0>(ret[2].get());
			std::get_if<8>(ret[3].get())->emplace_back
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
			auto const tag = *std::get_if<0>(ret[1].get());
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
				std::get_if<9>(ret[3].get())->try_emplace
				(	*std::get_if<7>(ret[1].get())
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
				std::get_if<9>(ret[3].get())->emplace
				(	*std::get_if<7>(ret[1].get())
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
			auto const count
				= *std::get_if<2>(ret.front().get());
			_Int_array_type cont;
			if (count > 0)
			{
				auto const len = count * 4;
				auto const buf
					= std::make_unique<_In_char[]>
					(	len
					);
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
			auto const count
				= *std::get_if<2>(ret.front().get());
			_Long_array_type cont;
			if (count > 0)
			{
				auto const len = count * 8;
				auto const buf
					= std::make_unique<_In_char[]>
					(	len
					);
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

# ifndef NBT_NO_BOOST
NBT_PARSER_TEMPLATE_DECLARATION
std::unique_ptr<_Node>
parse_auto
(	std::basic_istream<_In_char> &in
,	_Allocator<_Node> __a = _Allocator<_Node>()
)
{
	namespace io = boost::iostreams;
	typename std::char_traits<_In_char>::int_type tmp;
	switch (in.peek())
	{
	case 0x1F: // Maybe gzip
		in.get();
		tmp = in.peek();
		in.unget();
		if (tmp == 0x8B)
		{ // Assume gzip
			io::filtering_stream
			<	io::input
			,	_In_char
			,	std::char_traits<_In_char>
			,	std::allocator<_In_char>
			,	io::public_
			> filtered;
			filtered.push(io::gzip_decompressor());
			filtered.push(in);
			return parse<_Policy, _Allocator, _Char, _Byte, _Short, _Int, _Long, _Float, _Double, _Byte_array, _String, _List, _Compound, _Int_array, _Long_array, _Byte_array_type, _String_type, _List_type, _Compound_type, _Int_array_type, _Long_array_type, _Node, _In_char>(filtered, __a);
		}
		else
			break;
	case 0x78: // Maybe zlib
		in.get();
		tmp = in.peek();
		in.unget();
		if (tmp == 0x01 || tmp == 0x9C || tmp == 0xDA)
		{ // Assume zlib
			io::filtering_stream
			<	io::input
			,	_In_char
			,	std::char_traits<_In_char>
			,	std::allocator<_In_char>
			,	io::public_
			> filtered;
			filtered.push(io::zlib_decompressor());
			filtered.push(in);
			return parse<_Policy, _Allocator, _Char, _Byte, _Short, _Int, _Long, _Float, _Double, _Byte_array, _String, _List, _Compound, _Int_array, _Long_array, _Byte_array_type, _String_type, _List_type, _Compound_type, _Int_array_type, _Long_array_type, _Node, _In_char>(filtered, __a);
		}
		else
			break;
	}
	return parse<_Policy, _Allocator, _Char, _Byte, _Short, _Int, _Long, _Float, _Double, _Byte_array, _String, _List, _Compound, _Int_array, _Long_array, _Byte_array_type, _String_type, _List_type, _Compound_type, _Int_array_type, _Long_array_type, _Node, _In_char>(in, __a);
}
# endif // !NBT_NO_BOOST

NBT_PARSER_TEMPLATE_DECLARATION
std::unique_ptr<_Node>
parse_str
(	std::string const &in
,	_Allocator<_Node> __a = _Allocator<_Node>()
)
{
	std::istringstream iss(in);
# ifndef NBT_NO_BOOST
	return parse_auto
# else
	return parse
# endif // !NBT_NO_BOOST
	<	_Policy
	,	_Allocator
	,	_Char
	,	_Byte
	,	_Short
	,	_Int
	,	_Long
	,	_Float
	,	_Double
	,	_Byte_array
	,	_String
	,	_List
	,	_Compound
	,	_Int_array
	,	_Long_array
	,	_Byte_array_type
	,	_String_type
	,	_List_type
	,	_Compound_type
	,	_Int_array_type
	,	_Long_array_type
	,	_Node
	,	decltype(iss)::char_type
	>(iss, __a);
}

NBT_PARSER_TEMPLATE_DECLARATION
std::unique_ptr<_Node>
parse_file
(	std::string const &path
,	_Allocator<_Node> __a = _Allocator<_Node>()
)
{
	std::ifstream ifs(path, std::ios::binary);
# ifndef NBT_NO_BOOST
	return parse_auto
# else
	return parse
# endif // !NBT_NO_BOOST
	<	_Policy
	,	_Allocator
	,	_Char
	,	_Byte
	,	_Short
	,	_Int
	,	_Long
	,	_Float
	,	_Double
	,	_Byte_array
	,	_String
	,	_List
	,	_Compound
	,	_Int_array
	,	_Long_array
	,	_Byte_array_type
	,	_String_type
	,	_List_type
	,	_Compound_type
	,	_Int_array_type
	,	_Long_array_type
	,	_Node
	,	decltype(ifs)::char_type
	>(ifs, __a);
}

} // nbt

# undef NBT_PARSER_TEMPLATE_DECLARATION

#endif // !NBT_PARSER_H_
