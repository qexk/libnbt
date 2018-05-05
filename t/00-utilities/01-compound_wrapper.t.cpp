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

#define NBT_UNIT_TEST_
#include "nbt_utilities.hpp"

#include "catch.hpp"

#include <array>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>

using compound_type = std::unordered_map
<	std::string
,	std::unique_ptr<void, void (*) (void *)>
>;

template <class _Variant, typename ..._Pairs>
auto
make_compound(_Pairs const &...args_pack)
{
	compound_type ret;
	if constexpr (sizeof...(_Pairs) > 0)
	{
		std::array args = {args_pack...};
		for (auto const &v : args)
		{
			compound_type::mapped_type value
			(	reinterpret_cast<void *>
				(	new _Variant(v.second)
				)
			,	[] (void *raw) {
					delete reinterpret_cast<_Variant *>
					(	raw
					);
				}
			);
			ret.emplace
			(	std::string(v.first)
			,	std::move(value)
			);
			(void)(value);
		}
	}
	return ret;
}

SCENARIO( "nbt::detail::compound_wrapper::iterator satisfies Iterator concept" )
{
	using iterator = nbt::detail::compound_wrapper
	<	std::variant<int, compound_type>
	,	compound_type
	,	std::string
	>::iterator;

	static_assert(std::is_move_constructible_v<iterator>);
	GIVEN( "an iterator `u`" )
	{
		INFO( "MoveConstructible" );
		auto const compound = make_compound<std::variant<int>>();
		iterator u(compound.begin());
		WHEN( "`iterator v` is move-constructed from `u`" )
		{
			iterator v(std::move(u));
			THEN( "`v` is the same as `u`" )
			{
				CHECK( v.it == compound.begin() );
			}
		}
		WHEN( "`iterator v` is move-assigned from `u`" )
		{
			iterator v = std::move(u);
			THEN( "`v` is the same as `u`" )
			{
				CHECK( v.it == compound.begin() );
			}
		}
	}

	static_assert(std::is_copy_constructible_v<iterator>);
	GIVEN( "an iterator `u`" )
	{
		INFO( "CopyConstructible" );
		auto const compound = make_compound<std::variant<int>>();;
		iterator u(compound.begin());
		WHEN( "`iterator v` is copy-constructed from `u`" )
		{
			iterator v(u);
			THEN( "`v` is the same as `u`, `u` unchanged" )
			{
				CHECK( v.it == u.it );
				CHECK( u.it == compound.begin() );
			}
		}
		WHEN( "`iterator v` is copy-assigned from `u`" )
		{
			iterator v = u;
			THEN( "`v` is the same as `u`, `u` unchanged" )
			{
				CHECK( v.it == u.it );
				CHECK( u.it == compound.begin() );
			}
		}
	}

	static_assert(std::is_copy_assignable_v<iterator>);
	GIVEN( "an iterator `u`" )
	{
		INFO( "CopyAssignable" );
		auto const compound = make_compound<std::variant<int>>();
		iterator u(compound.begin());
		WHEN( "`iterator v` is assigned from `u`" )
		{
			iterator v(compound.end());
			v = u;
			THEN( "`v` is the same as `u`, `u` unchanged" )
			{
				CHECK( v.it == u.it );
				CHECK( u.it == compound.begin() );
			}
		}
	}

	static_assert(std::is_destructible_v<iterator>);
	GIVEN( "an iterator `u`" )
	{
		INFO( "Destructible" );
		auto const compound = make_compound<std::variant<int>>();
		iterator u(compound.begin());
		THEN( "`u` can be destroyed" )
		{
			CHECK(( u.~iterator(), true ));
		}
	}

	static_assert(std::is_swappable_v<iterator>);
	GIVEN( "two iterators `u` and `v`" )
	{
		INFO( "Swappable" );
		auto const compound = make_compound<std::variant<int>>
		(	std::make_pair("un",   1)
		,	std::make_pair("deux", 2)
		);
		iterator u(compound.begin());
		iterator v(compound.end());
		REQUIRE_FALSE( u.it == v.it );
		WHEN( "<utility> `std::swap` is called through ADL" )
		{
			using std::swap;
			swap(u, v);
			THEN( "`u.it` and `v.it` have exchanged values" )
			{
				CHECK( u.it == compound.end() );
				CHECK( v.it == compound.begin() );
			}
		}
	}

	GIVEN( "the type `nbt::detail::compound_wrapper<...>::iterator`" )
	{
		using variant = std::variant<int, compound_type>;
		using compound_wrapper = nbt::detail::compound_wrapper
		<	variant
		,	compound_type
		,	std::string
		>;
		THEN( "`iterator` exposes the same type as `compound_wrapper`" )
		{
			CHECK
			(	std::is_same_v
				<	compound_wrapper::value_type
				,	compound_wrapper::iterator::value_type
				>
			);
		}
		AND_THEN( "`value_type` defaults to the right type" )
		{
			CHECK
			(	std::is_same_v
				<	compound_wrapper::iterator::value_type
				,	std::pair
					<	std::string const
					,	variant const *
					>
				>
			);
		}
		INFO( "std::iterator_traits conformance" );
		WHEN( "using std::iterator_traits" )
		{
			using traits = std::iterator_traits<iterator>;
			THEN( "expected member types are defined" )
			{
				CHECK( std::is_same_v<iterator::difference_type, traits::difference_type> );
				CHECK( std::is_same_v<iterator::value_type, traits::value_type> );
				CHECK( std::is_same_v<iterator::pointer, traits::pointer> );
				CHECK( std::is_same_v<iterator::reference, traits::reference> );
				CHECK( std::is_same_v<iterator::iterator_category, traits::iterator_category> );
			}
		}
	}

	GIVEN( R"(an iterator `u` and a container `cont{{"un", 1}, {"deux", 2}, {"trois", 3}}`)" )
	{
		using variant = std::variant<int, compound_type>;
		auto const compound = make_compound<variant>
		(	std::make_pair("un",    1)
		,	std::make_pair("deux",  2)
		,	std::make_pair("trois", 3)
		);
		nbt::detail::compound_wrapper
		<	variant
		,	compound_type
		,	std::string
		>::iterator u;
		INFO( "Iterator expressions `*u`" );
		WHEN( "`u` and `v` are initialized to `cont.begin()`" )
		{
			u = compound.begin();
			THEN( "`*u` returns the first element" )
			{
				CHECK
				(	(*u).first
					== compound.begin()->first
				);
				CHECK
				(	(*u).second
					== compound.begin()->second.get()
				);
			}
			AND_THEN( "`*++u` returns the second element" )
			{
				CHECK
				(	(*++u).first
					== (++compound.begin())->first
				);
			}
			AND_THEN( "`*++u` returns the second element" )
			{

				CHECK
				(	(*++u).second
					== (++compound.begin())->second.get()
				);
			}
		}
	}
}

SCENARIO( "nbt::detail::compound_wrapper::iterator satisfies InputIterator concept" )
{
	using variant = std::variant
	<	std::monostate
	,	std::monostate
	,	int
	,	compound_type
	>;
	using iterator = nbt::detail::compound_wrapper
	<	variant
	,	compound_type
	,	std::string
	>::iterator;

	GIVEN( "three iterators `a`, `b` and `c` at `cont.begin()`" )
	{
		INFO( "EqualityComparable" );
		auto const compound = make_compound<variant>
		(	std::make_pair("un",    1)
		,	std::make_pair("deux",  2)
		,	std::make_pair("trois", 3)
		);
		iterator a(compound.begin());
		iterator b(compound.begin());
		iterator c(compound.begin());
		THEN( "they are all equal" )
		{
			CHECK( a == a );
			CHECK(( a == b && b == a ));
			CHECK(( a == b && b == c && a == c ));
		}
		WHEN( "`a` is incremented" )
		{
			++a;
			THEN( "a is equal to itself, but not to `b` nor `c`" )
			{
				INFO( "InputIterator `!=` expression" );
				CHECK( a == a );
				CHECK( a != b );
				CHECK( a != c );
			}
		}
		WHEN( "`a`, `b` and `c` are incremented" )
		{
			++a;
			++b;
			++c;
			THEN( "they are all equal, and dereference correctly" )
			{
				CHECK(( a == b && b == c && a == c ));
				CHECK( *a == *b );
				CHECK( *b == *c );
				CHECK( *c == *a );
			}
		}
	}

	GIVEN( "an iterator `a`" )
	{
		INFO( "`(void) *i, *i` is equivalent to `*i`" );
		auto const compound = make_compound<variant>
		(	std::make_pair("un",    1)
		);
		iterator a(compound.begin());
		THEN( "`*a` is pure" )
		{
			CHECK( ((void) *a, *a) == *a );
		}
	}

	GIVEN( "an iterator `a`" )
	{
		using variant = std::variant<std::string, compound_type>;
		using iterator = nbt::detail::compound_wrapper
		<	variant
		,	compound_type
		,	std::string
		>::iterator;
		INFO( "`i->m` is equivalent to `(*i).m`" );
		auto const compound = make_compound<variant>
		(	std::make_pair("Shrek",           "2001")
		,	std::make_pair("Shrek 2",         "2004")
		,	std::make_pair("Shrek the Third", "2007")
		);
		iterator a(compound.begin());
		THEN( "`a->m` allows the use of member `m`" )
		{
			CHECK( a->first == compound.begin()->first );
			CHECK
			(	a->second
				== compound.begin()->second.get()
			);
			CHECK( a->first.substr(0, 5) == "Shrek" );
		}
	}

	GIVEN( "an iterator `a`" )
	{
		INFO( "iterator expression `a++`" );
		auto const compound = make_compound<variant>
		(	std::make_pair("un",    1)
		,	std::make_pair("deux",  2)
		,	std::make_pair("trois", 3)
		);
		iterator a(compound.begin());
		THEN( "`a++` returns a copy of itself before incrementing" )
		{
			CHECK
			(	(a++)->second
				== compound.begin()->second.get()
			);
			CHECK
			(	a->second
				== (++compound.begin())->second.get()
			);
		}
	}
}

SCENARIO( "nbt::detail::compound_wrapper::iterator satisfies ForwardIterator concept" )
{
	using variant = std::variant<int, compound_type>;
	using iterator = nbt::detail::compound_wrapper
	<	variant
	,	compound_type
	,	std::string
	>::iterator;

	static_assert(std::is_default_constructible_v<iterator>);
	THEN( "`iterator` satisfies DefaultConstructible" )
	{
		INFO( "DefaultConstructible" );
		GIVEN( "two iterators `a` and `b` default-initialized" )
		{
			iterator a;
			iterator b;
			THEN( "they are equal" )
			{
				CHECK( a == b );
			}
		}

		GIVEN( "two iterators `a` and `b` value-initialized" )
		{
			iterator a{};
			iterator b{};
			THEN( "they are equal" )
			{
				CHECK( a == b );
			}
		}

		THEN( "two temporaries default-initialized are equivalent" )
		{
			CHECK( iterator() == iterator() );
		}

		THEN( "two temporaries value-initialized are equivalent" )
		{
			CHECK( iterator{} == iterator{} );
		}
	}

	GIVEN( "two iterators `a` and `b`" )
	{
		INFO( "multipass garantee" );
		auto const compound = make_compound<variant>
		(	std::make_pair("un",    1)
		,	std::make_pair("deux",  2)
		,	std::make_pair("trois", 3)
		);
		iterator a(compound.begin());
		iterator b(compound.begin());
		WHEN( "`a` is equivalent to `b`" )
		{
			CHECK( a == b );
			THEN( "incrementing both keeps them equivalent" )
			{
				CHECK( ++a == ++b );
			}
		}
		WHEN( "incrementing a copy of `a`" )
		{
			THEN( "equivalence is preserved" )
			{
				CHECK( ((void)++iterator(a), *a) == *a );
			}
		}
	}
}

SCENARIO( "nbt::detail::compound_wrapper satisfies Container concept" )
{
	using variant = std::variant<int, compound_type>;
	using compound_wrapper = nbt::detail::compound_wrapper
	<	variant
	,	compound_type
	,	std::string
	>;

	GIVEN( "a `compound_wrapper` type" )
	{
		INFO( "Types" );
		CHECK
		(	std::is_same_v
			<	compound_wrapper::reference
			,	std::add_lvalue_reference_t
				<	compound_wrapper::value_type
				>
			>
		);
		CHECK
		(	std::is_same_v
			<	compound_wrapper::const_reference
			,	std::add_lvalue_reference_t
				<	std::add_const_t
					<	compound_wrapper::value_type
					>
				>
			>
		);
		CHECK
		(	std::is_same_v
			<	compound_wrapper::const_iterator
			,	std::add_const_t
				<	compound_wrapper::iterator
				>
			>
		);
		CHECK( std::is_signed_v<compound_wrapper::difference_type> );
		CHECK
		(	std::is_same_v
			<	compound_wrapper::difference_type
			,	std::iterator_traits<compound_wrapper::iterator>
				::difference_type
			>
		);
		CHECK( std::is_unsigned_v<compound_wrapper::size_type> );
		CHECK
		(	std::numeric_limits<compound_wrapper::size_type>
				::max()
			> std::numeric_limits<compound_wrapper::difference_type>
				::max()
		);
	}

	INFO( "methods and expressions" )
	GIVEN( "a default-constructed compound_wrapper `c`" )
	{
		INFO( ".empty()" );
		compound_wrapper c;
		THEN( "`c` is empty" )
		{
			CHECK( c.empty() );
		}
		AND_THEN( "`c` can be copied" )
		{
			CHECK( compound_wrapper(c) == c );
		}
	}

	GIVEN( "two compound_wrappers `a` and `b` wrapping different containers" )
	{
		INFO( "assignment" );
		auto const compound_a = make_compound<variant>
		(	std::make_pair("un"   , 1)
		,	std::make_pair("deux" , 2)
		,	std::make_pair("trois", 3)
		);
		auto const compound_b = make_compound<variant>
		(	std::make_pair("quatre", 4)
		,	std::make_pair("cinq"  , 5)
		,	std::make_pair("six"   , 6)
		);
		compound_wrapper a(&compound_a);
		compound_wrapper b(&compound_b);
		WHEN( "`b` is assigned to `a`" )
		{
			a = b;
			THEN( "`a` and `b` are equivalent" )
			{
				CHECK( a == b );
				CHECK( a.cont == b.cont );
			}
		}
		WHEN( "`compound_wrapper rvalue` is assigned to `a`" )
		{
			a = compound_wrapper(&compound_b);
			THEN( "`a` and `b` are equivalent" )
			{
				CHECK( a == b );
				CHECK( a.cont == b.cont );
			}
		}
	}

	GIVEN( "a compound_wrapper `a`" )
	{
		INFO( "iterator types" );
		compound_wrapper a;
		THEN( "functions returning iterators actually return iterators" )
		{
			CHECK
			(	std::is_same_v
				<	decltype(a.begin())
				,	compound_wrapper::const_iterator
				>
			);
			CHECK
			(	std::is_same_v
				<	decltype(a.end())
				,	compound_wrapper::const_iterator
				>
			);
			CHECK
			(	std::is_same_v
				<	decltype(a.cbegin())
				,	compound_wrapper::const_iterator
				>
			);
			CHECK
			(	std::is_same_v
				<	decltype(a.cend())
				,	compound_wrapper::const_iterator
				>
			);
		}
	}

	GIVEN( "two compound_wrapper `a` and `b` of different template types wrapping different compounds containing the same sequence" )
	{
		INFO( "equivalence" );
		auto compound = make_compound<variant>
		(	std::make_pair("un"   , 1)
		,	std::make_pair("deux" , 2)
		,	std::make_pair("trois", 3)
		);
		compound_wrapper a(&compound);
		compound_wrapper b(&compound);
		THEN( "equality is maintained only when wrapping the same container" )
		{
			CHECK( a == b );
		}
		AND_WHEN( "`b`'s wrapped compound is changed" )
		{
			auto compound_b = make_compound<variant>
			(	std::make_pair("un"   , 1)
			,	std::make_pair("deux" , 2)
			,	std::make_pair("trois", 3)
			);
			b = &compound_b;
			THEN( "`b` is no longer equivalent to `a`" )
			{
				CHECK( a != b );
			}
		}
	}

	GIVEN( "two compound_wrappers `a` and `b` wrapping different containers" )
	{
		INFO( "swaps" );
		auto const compound_a = make_compound<variant>
		(	std::make_pair("un"   , 1)
		,	std::make_pair("deux" , 2)
		,	std::make_pair("trois", 3)
		);
		auto const compound_b = make_compound<variant>
		(	std::make_pair("quatre", 4)
		,	std::make_pair("cinq"  , 5)
		,	std::make_pair("six"   , 6)
		);
		compound_wrapper a(&compound_a);
		compound_wrapper b(&compound_b);
		compound_wrapper old_a(a);
		compound_wrapper old_b(b);
		REQUIRE( a != b );
		WHEN( "`a` is swapped with `b` using method" )
		{
			a.swap(b);
			THEN( "`a` and `b`'s wrappee are swapped" )
			{
				CHECK( a == old_b );
				CHECK( b == old_a );
			}
		}
		WHEN( "`a` is swapped with `b` using std::swap" )
		{
			using std::swap;
			swap(a, b);
			THEN( "`a` and `b`'s wrappee are swapped" )
			{
				CHECK( a == old_b );
				CHECK( b == old_a );
			}
		}
	}

	GIVEN( "a compound_wrapper `a`" )
	{
		compound_wrapper a;
		WHEN( "`{}` is assigned to `a`" )
		{
			auto const compound = make_compound<variant>();
			a = &compound;
			THEN( "`a` is empty" )
			{
				CHECK( a.empty() );
				CHECK
				(	a.size()
					== std::distance(a.begin(), a.end())
				);
				CHECK( a.size() == 0 );
			}
			AND_THEN( "`.max_size()` is the same for `a` and the sequence" )
			{
				CHECK( a.max_size() == compound.max_size() );
			}
		}
		WHEN( R"(`{{"un", 1}, {"deux", 2}, {"trois", 3}}` is assigned to `a`)" )
		{
			auto const compound = make_compound<variant>
			(	std::make_pair("un"   , 1)
			,	std::make_pair("deux" , 2)
			,	std::make_pair("trois", 3)
			);
			a = &compound;
			THEN( "`a` contains elements" )
			{
				CHECK_FALSE( a.empty() );
				CHECK
				(	a.size()
					== std::distance(a.begin(), a.end())
				);
				CHECK( a.size() == 3 );
			}
		}
	}
}

SCENARIO( "nbt::detail::compound_wrapper satisfies some SequenceContainer concepts" )
{
	using variant = std::variant
	<	std::monostate
	,	std::monostate
	,	int
	,	compound_type
	>;
	using compound_wrapper = nbt::detail::compound_wrapper
	<	variant
	,	compound_type
	,	std::string
	>;

	GIVEN( "a compound_wrapper `a`" )
	{
		INFO( "SequenceContainer" );
		compound_wrapper a;
		WHEN( R"(`{{"un", 1}, {"deux", 2}, {"trois", 3}}` is assigned to `a`)" )
		{
			auto const compound = make_compound<variant>
			(	std::make_pair("un"   , 1)
			,	std::make_pair("deux" , 2)
			,	std::make_pair("trois", 3)
			);
			std::unordered_map
			<	std::string
			,	int
			> const map =
			{	{ "un"   , 1 }
			,	{ "deux" , 2 }
			,	{ "trois", 3 }
			};
			a = &compound;
			AND_THEN( "`a[...]` and `a.at(...)` can be called" )
			{
				CHECK( nbt::int_(a["un"   ]) == 1 );
				CHECK( nbt::int_(a["deux" ]) == 2 );
				CHECK( nbt::int_(a["trois"]) == 3 );
				CHECK( nbt::int_(a.at("un"   )) == 1 );
				CHECK( nbt::int_(a.at("deux" )) == 2 );
				CHECK( nbt::int_(a.at("trois")) == 3 );
			}
			AND_THEN( "`a.at(x)` throws when `x >= a.size()`" )
			{
				CHECK_THROWS_AS
				(	a.at("DOUZE")
				,	std::out_of_range
				);
			}
		}
	}
}

SCENARIO( "nbt::detail::compound_wrapper satisfies some UnorderedAssociativeContainer concepts" )
{
	using variant = std::variant<int, compound_type>;
	using compound_wrapper = nbt::detail::compound_wrapper
	<	variant
	,	compound_type
	,	std::string
	>;
	GIVEN( "a `compound_wrapper` type" )
	{
		INFO( "Types" );
		CHECK
		(	std::is_same_v
			<	compound_wrapper::hasher
			,	compound_type::hasher
			>
		);
		CHECK
		(	std::is_same_v
			<	compound_wrapper::key_equal
			,	compound_type::key_equal
			>
		);
	}
}
