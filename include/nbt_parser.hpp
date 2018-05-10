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

# include "nbt_byteconverters.hpp"

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

enum class state
{	_1, S1
,	_2, S2
,	_3, S3
,	_4, S4
,	_5, S5
,	_6, S6
,	_7, S7, S7A
,	_8, S8, S8A
,	_9, S9, S9A, S9B
,	_A, SA, SAA, NT, SAB, SAEND
,	_B, SB, SBA
,	_C, SC, SCA
,	S, NTS // these states are parser-only
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
	default:    return state::F;
	}
}

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
	,	state
	>;
	std::unordered_map<state, _Byte_to_case> const trans =
	{	{ state::F, {{ _In_traits::eof(), state::F }} }
	,	{	state::S
		,	{	{ tag_byt, state::_1 }
			,	{ tag_sht, state::_2 }
			,	{ tag_int, state::_3 }
			,	{ tag_lng, state::_4 }
			,	{ tag_flt, state::_5 }
			,	{ tag_dbl, state::_6 }
			,	{ tag_bya, state::_7 }
			,	{ tag_str, state::_8 }
			,	{ tag_lst, state::_9 }
			,	{ tag_cpd, state::_A }
			,	{ tag_ina, state::_B }
			,	{ tag_lna, state::_C }
			}
		}
	,	{ state::S1,  {{ detail::_, state::S1  }} }
	,	{ state::S2,  {{ detail::_, state::S2  }} }
	,	{ state::S3,  {{ detail::_, state::S3  }} }
	,	{ state::S4,  {{ detail::_, state::S4  }} }
	,	{ state::S5,  {{ detail::_, state::S5  }} }
	,	{ state::S6,  {{ detail::_, state::S6  }} }
	,	{ state::S7,  {{ detail::_, state::S7  }} }
	,	{ state::S7A, {{ detail::_, state::S7A }} }
	,	{ state::S8,  {{ detail::_, state::S8  }} }
	,	{ state::S8A, {{ detail::_, state::S8A }} }
	,	{ state::S9,  {{ detail::_, state::S9  }} }
	,	{ state::S9A, {{ detail::_, state::S9A }} }
	,	{ state::S9B, {{ detail::_, state::S9B }} }
	,	{ state::SA,  {{ detail::_, state::SA  }} }
	,	{ state::SAA, {{ detail::_, state::SAB }} }
	,	{ state::SB,  {{ detail::_, state::SB  }} }
	,	{ state::SBA, {{ detail::_, state::SBA }} }
	,	{ state::SC,  {{ detail::_, state::SC  }} }
	,	{ state::SCA, {{ detail::_, state::SCA }} }
	,	{ state::NT,  {{ detail::_, state::NT  }} }
	,	{	state::NTS
		,	_Policy == parsing::implicit_compound
			?	_Byte_to_case
				{	{ tag_nul,           state::SAEND }
				,	{ _In_traits::eof(), state::F     }
				,	{ detail::_,         state::SAA   }
				}
			:	_Byte_to_case
				{	{ tag_nul,   state::SAEND }
				,	{ detail::_, state::SAA   }
				}
		}
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
	case state::_1:
		in.get();
		ss.pop();
		ss.push(state::S1);
		goto loop;
	case state::S1:
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
	case state::_2:
		in.get();
		ss.pop();
		ss.push(state::S2);
		goto loop;
	case state::S2:
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
	case state::_3:
		in.get();
		ss.pop();
		ss.push(state::S3);
		goto loop;
	case state::S3:
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
	case state::_4:
		in.get();
		ss.pop();
		ss.push(state::S4);
		goto loop;
	case state::S4:
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
	case state::_5:
		in.get();
		ss.pop();
		ss.push(state::S5);
		goto loop;
	case state::S5:
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
	case state::_6:
		in.get();
		ss.pop();
		ss.push(state::S6);
		goto loop;
	case state::S6:
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
	case state::_7:
		in.get();
		ss.pop();
		ss.push(state::S7);
		goto loop;
	case state::S7:
		ss.pop();
		ss.push(state::S7A);
		ss.push(state::S3);
		goto loop;
	case state::S7A:
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
	case state::_8:
		in.get();
		ss.pop();
		ss.push(state::S8);
		goto loop;
	case state::S8:
		ss.pop();
		ss.push(state::S8A);
		ss.push(state::S2);
		goto loop;
	case state::S8A:
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
	case state::_9:
		in.get();
		ss.pop();
		ss.push(state::S9);
		goto loop;
	case state::S9:
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
	case state::S9A:
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
	case state::S9B:
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
	case state::_A:
		in.get();
		ss.pop();
		ss.push(state::SA);
		goto loop;
	case state::SA:
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
	case state::SAA:
		ss.pop();
		ss.push(state::NT);
		ss.push(state::S8);
		ss.push(state::S1);
		goto loop;
	case state::NT:
		{
			auto const tag = *std::get_if<0>(ret[1].get());
			ss.pop();
			ss.push(state::SAA);
			ss.push(detail::state_of_tag(tag));
			goto loop;
		}
	case state::SAB:
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
	case state::SAEND:
		in.get();
		ss.pop();
		goto loop;
	case state::_B:
		in.get();
		ss.pop();
		ss.push(state::SB);
		goto loop;
	case state::SB:
		ss.pop();
		ss.push(state::SBA);
		ss.push(state::S3);
		goto loop;
	case state::SBA:
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
	case state::_C:
		in.get();
		ss.pop();
		ss.push(state::SC);
		goto loop;
	case state::SC:
		ss.pop();
		ss.push(state::SCA);
		ss.push(state::S3);
		goto loop;
	case state::SCA:
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
	case state::S:
	case state::NTS:
	case state::F:
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
	<	_Policy, _Allocator, _Char, _Byte, _Short, _Int, _Long, _Float, _Double, _Byte_array, _String, _List, _Compound, _Int_array, _Long_array, _Byte_array_type, _String_type, _List_type, _Compound_type, _Int_array_type, _Long_array_type, _Node
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
	<	_Policy, _Allocator, _Char, _Byte, _Short, _Int, _Long, _Float, _Double, _Byte_array, _String, _List, _Compound, _Int_array, _Long_array, _Byte_array_type, _String_type, _List_type, _Compound_type, _Int_array_type, _Long_array_type, _Node
	,	decltype(ifs)::char_type
	>(ifs, __a);
}

} // nbt

# undef NBT_PARSER_TEMPLATE_DECLARATION

#endif // !NBT_PARSER_H_
