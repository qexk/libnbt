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

SCENARIO( "nbt::detail::list_wrapper::iterator satisfies ForwardIterator concept" )
{
	using variant = std::variant<int, list_type>;
	using iterator = nbt::detail::list_wrapper
	<	0
	,	variant
	,	list_type
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

	THEN( "`iterator` is a non-mutable InputIterator" )
	{
		CHECK
		(	std::is_same_v
			<	std::iterator_traits<iterator>::reference
			,	std::add_lvalue_reference_t
				<	std::add_const_t
					<	std::iterator_traits
						<	iterator
						>::value_type
					>
				>
			>
		);
	}

	GIVEN( "two iterators `a` and `b`" )
	{
		INFO( "multipass garantee" );
		auto const list = make_list<variant>(42, 84);
		iterator a(list.begin());
		iterator b(list.begin());
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

SCENARIO( "nbt::detail::list_wrapper satisfies Container concept" )
{
	using variant = std::variant<int, list_type>;
	using list_wrapper = nbt::detail::list_wrapper
	<	0
	,	variant
	,	list_type
	>;

	GIVEN( "a `list_wrapper` type" )
	{
		INFO( "Types" );
		CHECK
		(	std::is_same_v
			<	list_wrapper::reference
			,	std::add_lvalue_reference_t
				<	list_wrapper::value_type
				>
			>
		);
		CHECK
		(	std::is_same_v
			<	list_wrapper::const_reference
			,	std::add_lvalue_reference_t
				<	std::add_const_t
					<	list_wrapper::value_type
					>
				>
			>
		);
		CHECK
		(	std::is_same_v
			<	list_wrapper::const_iterator
			,	std::add_const_t
				<	list_wrapper::iterator
				>
			>
		);
		CHECK( std::is_signed_v<list_wrapper::difference_type> );
		CHECK
		(	std::is_same_v
			<	list_wrapper::difference_type
			,	std::iterator_traits<list_wrapper::iterator>
				::difference_type
			>
		);
		CHECK( std::is_unsigned_v<list_wrapper::size_type> );
		CHECK
		(	std::numeric_limits<list_wrapper::size_type>
				::max()
			> std::numeric_limits<list_wrapper::difference_type>
				::max()
		);
	}

	INFO( "methods and expressions" )
	GIVEN( "a default-constructed list_wrapper `c`" )
	{
		INFO( ".empty()" );
		list_wrapper c;
		THEN( "`c` is empty" )
		{
			CHECK( c.empty() );
		}
		AND_THEN( "`c` can be copied" )
		{
			CHECK( list_wrapper(c) == c );
		}
	}

	GIVEN( "two list_wrappers `a` and `b` wrapping different containers" )
	{
		INFO( "assignment" );
		auto const list_a = make_list<variant>(1, 2, 3);
		auto const list_b = make_list<variant>(4, 5, 6);
		list_wrapper a(&list_a);
		list_wrapper b(&list_b);
		WHEN( "`b` is assigned to `a`" )
		{
			a = b;
			THEN( "`a` and `b` are equivalent" )
			{
				CHECK( a == b );
				CHECK( a.cont == b.cont );
			}
		}
		WHEN( "`list_wrapper rvalue` is assigned to `a`" )
		{
			a = list_wrapper(&list_b);
			THEN( "`a` and `b` are equivalent" )
			{
				CHECK( a == b );
				CHECK( a.cont == b.cont );
			}
		}
	}

	GIVEN( "a list_wrapper `a`" )
	{
		INFO( "iterator types" );
		list_wrapper a;
		THEN( "functions returning iterators actually return iterators" )
		{
			CHECK
			(	std::is_same_v
				<	decltype(a.begin())
				,	list_wrapper::const_iterator
				>
			);
			CHECK
			(	std::is_same_v
				<	decltype(a.end())
				,	list_wrapper::const_iterator
				>
			);
			CHECK
			(	std::is_same_v
				<	decltype(a.cbegin())
				,	list_wrapper::const_iterator
				>
			);
			CHECK
			(	std::is_same_v
				<	decltype(a.cend())
				,	list_wrapper::const_iterator
				>
			);
		}
	}

	GIVEN( "two list_wrapper `a` and `b` of different template types wrapping different lists containing the same sequence" )
	{
		INFO( "equivalence" );
		using variant_a = std::variant<std::uint16_t, list_type>;
		using variant_b = std::variant<std::uint32_t, list_type>;
		auto list_a = make_list<variant_a>(1, 2, 3);
		auto list_b = make_list<variant_b>(1, 2, 3);
		nbt::detail::list_wrapper
		<	0
		,	variant_a
		,	list_type
		> a(&list_a);
		nbt::detail::list_wrapper
		<	0
		,	variant_b
		,	list_type
		> b(&list_b);
		THEN( "equality is maintained across different types" )
		{
			CHECK( a == b );
		}
		AND_WHEN( "`b`'s wrapped list is changed" )
		{
			list_b = make_list<variant_b>(1, 2, 0);
			THEN( "`b` is no longer equivalent to `a`" )
			{
				CHECK( a != b );
			}
		}
	}

	GIVEN( "two list_wrappers `a` and `b` wrapping different containers" )
	{
		INFO( "swaps" );
		auto const list_a = make_list<variant>(1, 2, 3);
		auto const list_b = make_list<variant>(4, 5, 6);
		list_wrapper a(&list_a);
		list_wrapper b(&list_b);
		list_wrapper old_a(a);
		list_wrapper old_b(b);
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

	GIVEN( "a list_wrapper `a`" )
	{
		list_wrapper a;
		WHEN( "`{}` is assigned to `a`" )
		{
			auto const list = make_list<variant>();
			a = &list;
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
				CHECK( a.max_size() == list.max_size() );
			}
		}
		WHEN( "`{1, 2, 3}` is assigned to `a`" )
		{
			auto const list = make_list<variant>(1, 2, 3);
			a = &list;
			THEN( "`a` is empty" )
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

SCENARIO( "nbt::detail::list_wrapper satisfies some SequenceContainer concepts" )
{
	using variant = std::variant<int, list_type>;
	using list_wrapper = nbt::detail::list_wrapper
	<	0
	,	variant
	,	list_type
	>;

	GIVEN( "a list_wrapper `a`" )
	{
		INFO( "SequenceContainer" );
		list_wrapper a;
		WHEN( "`{1, 2, 3}` is assigned to `a`" )
		{
			auto const list = make_list<variant>(1, 2, 3);
			a = &list;
			THEN( "`a.back()` and `a.back()` can be called" )
			{
				CHECK( a.front() == 1 );
				CHECK( a.back() == 3 );
			}
			AND_THEN( "`a[...]` and `a.at(...)` can be called" )
			{
				CHECK( a[0] == 1 );
				CHECK( a[1] == 2 );
				CHECK( a[2] == 3 );
				CHECK( a.at(0) == 1 );
				CHECK( a.at(1) == 2 );
				CHECK( a.at(2) == 3 );
			}
			AND_THEN( "`a.at(x)` throws when `x >= a.size()`" )
			{
				CHECK_THROWS_AS
				(	a.at(a.size())
				,	std::out_of_range
				);
			}
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
