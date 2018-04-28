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

#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_FAST_COMPILE
#define CATCH_CONFIG_DISABLE_MATCHERS
#define CATCH_CONFIG_DEFAULT_REPORTER "tap"
#include "catch.hpp"
#include "catch_reporter_tap.hpp"

#include <array>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

using list_type = std::vector<std::unique_ptr<void, void (*) (void *)>>;

template <class _Variant, typename ..._Args>
auto
make_list(_Args ...args_pack)
{
	list_type ret;
	if constexpr (sizeof...(_Args) > 0)
	{
		std::array args = {args_pack...};
		for (auto const &v : args)
			ret.emplace_back
			(	reinterpret_cast<void *>(new _Variant(v))
			,	[] (void *raw) {
					delete reinterpret_cast<_Variant *>
					(	raw
					);
				}
			);
	}
	return ret;
}

SCENARIO( "nbt::detail::list_wrapper::iterator satisfies Iterator concept" )
{
	using iterator = nbt::detail::list_wrapper
	<	0
	,	std::variant<int, list_type>
	,	list_type
	>::iterator;

	static_assert(std::is_move_constructible_v<iterator>);
	GIVEN( "an iterator `u`" )
	{
		INFO( "MoveConstructible" );
		auto const list = make_list<std::variant<int>>();
		iterator u(list.begin());
		WHEN( "`iterator v` is move-constructed from `u`" )
		{
			iterator v(std::move(u));
			THEN( "`v` is the same as `u`" )
			{
				CHECK( v.it == list.begin() );
			}
		}
		WHEN( "`iterator v` is move-assigned from `u`" )
		{
			iterator v = std::move(u);
			THEN( "`v` is the same as `u`" )
			{
				CHECK( v.it == list.begin() );
			}
		}
	}

	static_assert(std::is_copy_constructible_v<iterator>);
	GIVEN( "an iterator `u`" )
	{
		INFO( "CopyConstructible" );
		auto const list = make_list<std::variant<int>>();;
		iterator u(list.begin());
		WHEN( "`iterator v` is copy-constructed from `u`" )
		{
			iterator v(u);
			THEN( "`v` is the same as `u`, `u` unchanged" )
			{
				CHECK( v.it == u.it );
				CHECK( u.it == list.begin() );
			}
		}
		WHEN( "`iterator v` is copy-assigned from `u`" )
		{
			iterator v = u;
			THEN( "`v` is the same as `u`, `u` unchanged" )
			{
				CHECK( v.it == u.it );
				CHECK( u.it == list.begin() );
			}
		}
	}

	static_assert(std::is_copy_assignable_v<iterator>);
	GIVEN( "an iterator `u`" )
	{
		INFO( "CopyAssignable" );
		auto const list = make_list<std::variant<int>>();
		iterator u(list.begin());
		WHEN( "`iterator v` is assigned from `u`" )
		{
			iterator v(list.end());
			v = u;
			THEN( "`v` is the same as `u`, `u` unchanged" )
			{
				CHECK( v.it == u.it );
				CHECK( u.it == list.begin() );
			}
		}
	}

	static_assert(std::is_destructible_v<iterator>);
	GIVEN( "an iterator `u`" )
	{
		INFO( "Destructible" );
		auto const list = make_list<std::variant<int>>();
		iterator u(list.begin());
		THEN( "`u` can be destroyed" )
		{
			CHECK(( u.~iterator(), true ));
		}
	}

	static_assert(std::is_swappable_v<iterator>);
	GIVEN( "two iterators `u` and `v`" )
	{
		INFO( "Swappable" );
		auto const list = make_list<std::variant<int>>(0, 1, 2);
		iterator u(list.begin());
		iterator v(list.end());
		REQUIRE_FALSE( u.it == v.it );
		WHEN( "<utility> `std::swap` is called through ADL" )
		{
			using std::swap;
			swap(u, v);
			THEN( "`u.it` and `v.it` have exchanged values" )
			{
				CHECK( u.it == list.end() );
				CHECK( v.it == list.begin() );
			}
		}
	}

	GIVEN( "the type `nbt::detail::list_wrapper<...>::iterator`" )
	{
		using variant = std::variant<int, list_type>;
		using list_alt0 = nbt::detail::list_wrapper
		<	0
		,	variant
		,	list_type
		>;
		using list_alt_1 = nbt::detail::list_wrapper
		<	(unsigned)(-1)
		,	variant
		,	list_type
		>;
		THEN( "`iterator` exposes the same type as `list_wrapper`" )
		{
			CHECK
			(	std::is_same_v
				<	list_alt0::value_type
				,	list_alt0::iterator::value_type
				>
			);
			CHECK
			(	std::is_same_v
				<	list_alt_1::value_type
				,	list_alt_1::iterator::value_type
				>
			);
		}
		AND_THEN( "`value_type` defaults to the right type" )
		{
			CHECK
			(	std::is_same_v
				<	list_alt0::iterator::value_type
				,	int const
				>
			);
			CHECK_FALSE( list_alt0::variant_or_default::is_default );
			CHECK
			(	std::is_same_v
				<	list_alt_1::iterator::value_type
				,	std::variant<int, list_type> const
				>
			);
			CHECK( list_alt_1::variant_or_default::is_default );
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

	GIVEN( "two iterators `u` and `v` and a container `cont{1, 2, 3}`" )
	{
		using variant = std::variant<int, list_type>;
		auto const list = make_list<variant>(1, 2, 3);
		nbt::detail::list_wrapper
		<	0
		,	variant
		,	list_type
		>::iterator u;
		nbt::detail::list_wrapper
		<	(unsigned)(-1)
		,	variant
		,	list_type
		>::iterator v;
		INFO( "Iterator expressions `*u`" );
		WHEN( "`u` and `v` are initialized to `cont.begin()`" )
		{
			u = list.begin();
			v = list.begin();
			THEN( "`*u` returns the first element" )
			{
				CHECK( *u == 1 );
				CHECK( std::get<0>(*v) == 1 );
			}
			AND_THEN( "`*++u` returns the second element" )
			{
				CHECK( *++u == 2 );
				CHECK( std::get<0>(*++v) == 2 );
			}
		}
	}
}
#include <variant>
SCENARIO( "nbt::detail::list_wrapper::iterator satisfies InputIterator concept" )
{
	using variant = std::variant<int, list_type>;
	using iterator = nbt::detail::list_wrapper
	<	0
	,	variant
	,	list_type
	>::iterator;

	GIVEN( "three iterators `a`, `b` and `c` at `cont.begin()`" )
	{
		INFO( "EqualityComparable" );
		auto const list = make_list<variant>(1, 2, 3);
		iterator a(list.begin());
		iterator b(list.begin());
		iterator c(list.begin());
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
		auto const list = make_list<variant>(42);
		iterator a(list.begin());
		THEN( "`*a` is pure" )
		{
			CHECK( ((void) *a, *a) == *a );
		}
	}


	GIVEN( "an iterator `a`" )
	{
		using variant = std::variant<std::string, list_type>;
		using iterator = nbt::detail::list_wrapper
		<	0
		,	variant
		,	list_type
		>::iterator;
		INFO( "`i->m` is equivalent to `(*i).m`" );
		auto const list = make_list<variant>("bonjour", "madame");
		iterator a(list.begin());
		THEN( "`a->m` allows the use of member `n`" )
		{
			CHECK( a->substr(0, 2) == "bo" );
			CHECK( (++a)->substr(0, 2) == "ma" );
		}
	}

	GIVEN( "an iterator `a`" )
	{
		INFO( "iterator expression `a++`" );
		auto const list = make_list<variant>(42, 84);
		iterator a(list.begin());
		THEN( "`a++` returns a copy of itself before incrementing" )
		{
			CHECK( *a++ == 42 );
			CHECK( *a == 84 );
		}
	}
}

int
main(int argc, char const * const *argv)
{
	Catch::Session session;
	if (auto ret = session.applyCommandLine(argc, argv))
		return ret;
	// Needed to correct TAP output suitable for prove
	session.configData().showSuccessfulTests = true;
	return session.run();
}
