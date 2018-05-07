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
#include "nbt.hpp"

#include "catch.hpp"

#include <algorithm>
#include <array>
#include <sstream>
#include <type_traits>
#include <variant>
#include <cmath>
#include <cstdint>

auto
make_stream(char const *s, std::size_t const len)
{
	return std::istringstream(std::string(s, len));
}

TEST_CASE( "parsing TAG_Byte" )
{
	SECTION( "valid TAG_Byte" )
	{
		std::array tests =
		{	std::pair("\x01""\x2a", 0x2A)
		,	std::pair("\x01""\x00", 0x00)
		,	std::pair("\x01""\xFF", 0xFF)
		};
		for (auto const &[s, expected] : tests)
		{
			auto iss = make_stream(s, 2);
			auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
			auto std_get = std::get<nbt::byte>(*res);
			auto nbt_fun = nbt::byte(res);
			CHECK
			(	std::is_convertible_v
				<	decltype(std_get)
				,	std::int8_t
				>
			);
			CHECK
			(	std::is_convertible_v
				<	decltype(nbt_fun)
				,	std::int8_t
				>
			);
			CHECK( std_get == nbt_fun );
			CHECK( std_get == (std::int8_t)(expected) );
			CHECK( nbt_fun == (std::int8_t)(expected) );
		}
	}
}

TEST_CASE( "parsing TAG_Short" )
{
	SECTION( "valid TAG_Short" )
	{
		std::array tests =
		{	std::pair("\x02""\x00\x2a", 42)
		,	std::pair("\x02""\x00\x00", 0)
		,	std::pair("\x02""\xFF\xFF", -1)
		};
		for (auto const &[s, expected] : tests)
		{
			auto iss = make_stream(s, 3);
			auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
			auto std_get = std::get<nbt::short_>(*res);
			auto nbt_fun = nbt::short_(res);
			CHECK
			(	std::is_convertible_v
				<	decltype(std_get)
				,	std::int16_t
				>
			);
			CHECK
			(	std::is_convertible_v
				<	decltype(nbt_fun)
				,	std::int16_t
				>
			);
			CHECK( std_get == nbt_fun );
			CHECK( std_get == (std::int16_t)(expected) );
			CHECK( nbt_fun == (std::int16_t)(expected) );
		}
	}
}

TEST_CASE( "parsing TAG_Int" )
{
	SECTION( "valid TAG_Int" )
	{
		std::array tests =
		{	std::pair("\x03""\x00\x00\x00\x2a", 42L)
		,	std::pair("\x03""\x00\x00\x00\x00", 0L)
		,	std::pair("\x03""\xFF\xFF\xFF\xFF", -1L)
		};
		for (auto const &[s, expected] : tests)
		{
			auto iss = make_stream(s, 5);
			auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
			auto std_get = std::get<nbt::int_>(*res);
			auto nbt_fun = nbt::int_(res);
			CHECK
			(	std::is_convertible_v
				<	decltype(std_get)
				,	std::int32_t
				>
			);
			CHECK
			(	std::is_convertible_v
				<	decltype(nbt_fun)
				,	std::int32_t
				>
			);
			CHECK( std_get == nbt_fun );
			CHECK( std_get == (std::int32_t)(expected) );
			CHECK( nbt_fun == (std::int32_t)(expected) );
		}
	}
}

TEST_CASE( "parsing TAG_Long" )
{
	SECTION( "valid TAG_Long" )
	{
		std::array tests =
		{	std::pair
			(	"\x04""\x00\x00\x00\x00\x00\x00\x00\x2a"
			,	42LL
			)
		,	std::pair
			(	"\x04""\x00\x00\x00\x00\x00\x00\x00\x00"
			,	0LL)

		,	std::pair
			(	"\x04""\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
			,	-1LL
			)
		};
		for (auto const &[s, expected] : tests)
		{
			auto iss = make_stream(s, 9);
			auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
			auto std_get = std::get<nbt::long_>(*res);
			auto nbt_fun = nbt::long_(res);
			CHECK
			(	std::is_convertible_v
				<	decltype(std_get)
				,	std::int64_t
				>
			);
			CHECK
			(	std::is_convertible_v
				<	decltype(nbt_fun)
				,	std::int64_t
				>
			);
			CHECK( std_get == nbt_fun );
			CHECK( std_get == (std::int64_t)(expected) );
			CHECK( nbt_fun == (std::int64_t)(expected) );
		}
	}
}

TEST_CASE( "parsing TAG_Float" )
{
	SECTION( "valid TAG_Float" )
	{
		std::array tests =
		{	std::pair("\x05""\x00\x00\x00\x00", 0.f)
		,	std::pair("\x05""\x80\x00\x00\x00", -0.f)
		,	std::pair("\x05""\x3F\x80\x00\x00", 1.f)
		,	std::pair("\x05""\x42\x2A\xA0\x00", 42.65625f)
		,	std::pair
			(	"\x05""\x7F\x80\x00\x00"
			,	std::numeric_limits<float>::infinity()
			)
		,	std::pair
			(	"\x05""\xFF\x80\x00\x00"
			,	-std::numeric_limits<float>::infinity()
			)
		,	std::pair
			(	"\x05""\x7F\x80\x00\x01"
			,	std::numeric_limits<float>::signaling_NaN()
			)
		,	std::pair
			(	"\x05""\x7F\xC0\x00\x00"
			,	std::numeric_limits<float>::quiet_NaN()
			)
		};
		for (auto const &[s, expected] : tests)
		{
			auto iss = make_stream(s, 5);
			auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
			auto std_get = std::get<nbt::float_>(*res);
			auto nbt_fun = nbt::float_(res);
			CHECK
			(	std::is_convertible_v
				<	decltype(std_get)
				,	float
				>
			);
			CHECK
			(	std::is_convertible_v
				<	decltype(nbt_fun)
				,	float
				>
			);
			if (std::isnan(expected))
			{
				CHECK( std::isnan(std_get) );
				CHECK( std::isnan(nbt_fun) );
			}
			else
			{
				CHECK( std_get == nbt_fun );
				CHECK( std_get == expected );
				CHECK( nbt_fun == expected );
			}
		}
	}
}

TEST_CASE( "parsing TAG_Double" )
{
	SECTION( "valid TAG_Double" )
	{
		std::array tests =
		{	std::pair
			(	"\x06""\x00\x00\x00\x00\x00\x00\x00\x00"
			,	0.
			)
		,	std::pair
			(	"\x06""\x80\x00\x00\x00\x00\x00\x00\x00"
			,	-0.
			)
		,	std::pair
			(	"\x06""\x3F\xF0\x00\x00\x00\x00\x00\x00"
			,	1.
			)
		,	std::pair
			(	"\x06""\x40\x45\x54\x00\x00\x00\x00\x00"
			,	42.65625
			)
		,	std::pair
			(	"\x06""\xD8\x64\xC3\xA1\x3A\x38\xBE\x50"
			,	-6545202440819396490979314559386213758545193276348547163361481275839832948327879231713265948162917543672258968084807680.0
			)
		,	std::pair
			(	"\x06""\x7F\xF0\x00\x00\x00\x00\x00\x00"
			,	std::numeric_limits<double>::infinity()
			)
		,	std::pair
			(	"\x06""\xFF\xF0\x00\x00\x00\x00\x00\x00"
			,	-std::numeric_limits<double>::infinity()
			)
		,	std::pair
			(	"\x06""\x7F\xF0\x00\x00\x00\x00\x00\x01"
			,	std::numeric_limits<double>::signaling_NaN()
			)
		,	std::pair
			(	"\x06""\x7F\xF8\x00\x00\x00\x00\x00\x00"
			,	std::numeric_limits<double>::quiet_NaN()
			)
		};
		for (auto const &[s, expected] : tests)
		{
			auto iss = make_stream(s, 9);
			auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
			auto std_get = std::get<nbt::double_>(*res);
			auto nbt_fun = nbt::double_(res);
			CHECK
			(	std::is_convertible_v
				<	decltype(std_get)
				,	float
				>
			);
			CHECK
			(	std::is_convertible_v
				<	decltype(nbt_fun)
				,	float
				>
			);
			if (std::isnan(expected))
			{
				CHECK( std::isnan(std_get) );
				CHECK( std::isnan(nbt_fun) );
			}
			else
			{
				CHECK( std_get == nbt_fun );
				CHECK( std_get == expected );
				CHECK( nbt_fun == expected );
			}
		}
	}
}

TEST_CASE( "parsing TAG_Byte_Array" )
{
	SECTION( "valid TAG_Byte_Array" )
	{
		using cont = std::vector<std::int_least8_t>;
		std::array tests =
		{	std::tuple
			(	"\x07""\x00\x00\x00\x00", 5
			,	cont{}
			)
		,	std::tuple
			(	"\x07""\xFF\xFF\xFF\xFF", 5
			,	cont{}
			)
		,	std::tuple
			(	"\x07""\x00\x00\x00\x07""covfefe", 12
			,	cont{'c', 'o', 'v' ,'f', 'e', 'f', 'e'}
			)
		};
		for (auto const &[s, len, expected] : tests)
		{
			auto iss = make_stream(s, len);
			auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
			auto std_get = std::get<nbt::byte_array>(*res);
			auto nbt_fun = nbt::byte_array(res);
			CHECK( std_get == nbt_fun );
			CHECK( std_get == expected );
			CHECK( nbt_fun == expected );
		}
	}
}

TEST_CASE( "parsing TAG_String" )
{
	SECTION( "valid TAG_String" )
	{
		std::array tests =
		{	std::tuple
			(	"\x08""\x00\x00", 3
			,	std::string("")
			)
		,	std::tuple
			(	"\x08""\x00\x14" u8"Pèlerin circonflexe", 23
			,	std::string(u8"Pèlerin circonflexe")
			)
		};
		for (auto const &[s, len, expected] : tests)
		{
			auto iss = make_stream(s, len);
			auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
			auto std_get = std::get<nbt::string>(*res);
			auto nbt_fun = nbt::string(res);
			CHECK( std_get == nbt_fun );
			CHECK( std_get == expected );
			CHECK( nbt_fun == expected );
		}
	}
}

TEST_CASE( "parsing TAG_List" )
{
	SECTION( "empty TAG_List" )
	{
		auto iss = make_stream
		(	"\x09""\x00""\x00\x00\x00\x00"
		,	6
		);
		auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
		auto &std_get = std::get<nbt::list>(*res);
		auto nbt_fun = nbt::list(res);
		CHECK( std_get.size() == 0 );
		CHECK( nbt_fun.size() == 0 );
	}
	SECTION( "TAG_List containing `{1, 2, 3}`" )
	{
		auto iss = make_stream
		(	"\x09""\x01""\x00\x00\x00\x03""\x01\x02\x03"
		,	9
		);
		auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
		auto &std_get = std::get<nbt::list>(*res);
		auto nbt_fun = nbt::list.as<nbt::byte>(res);
		CHECK( std_get.size() == 3 );
		CHECK( nbt_fun.size() == 3 );
		CHECK
		(	std::equal
			(	nbt_fun.begin(), nbt_fun.end()
			,	std::vector<int>{1, 2, 3}.begin()
			)
		);
		CHECK
		(	std::equal
			(	std_get.begin(), std_get.end()
			,	std::vector<int>{1, 2, 3}.begin()
			,	[] (auto const &l, auto const r) {
					return *reinterpret_cast<std::uint8_t *>
					(	l.get()
					) == r;
				}
			)
		);
	}
	SECTION( "TAG_List of TAG_List containing `{1, 2, 3}`" )
	{
		auto iss = make_stream
		(	"\x09""\x09""\x00\x00\x00\x03"
				"\x01""\x00\x00\x00\x03""\x01\x02\x03"
				"\x01""\x00\x00\x00\x03""\x01\x02\x03"
				"\x01""\x00\x00\x00\x03""\x01\x02\x03"
		,	30
		);
		auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
		auto &std_get = std::get<nbt::list>(*res);
		auto nbt_fun = nbt::list.as<nbt::list>(res);
		CHECK( std_get.size() == 3 );
		CHECK( nbt_fun.size() == 3 );
		CHECK
		(	std::all_of
			(	nbt_fun.begin(), nbt_fun.end()
			,	[] (auto const &v) {
					auto const l =
						nbt::list.as<nbt::byte>(v);
					return std::equal
					(	l.begin(), l.end()
					,	std::vector{1, 2, 3}.begin()
					);
				}
			)
		);
	}
}

TEST_CASE( "parsing TAG_Compound" )
{
	SECTION( "empty TAG_Compound" )
	{
		auto iss = make_stream("\x0A""\x00", 2);
		auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
		auto &std_get = std::get<nbt::compound>(*res);
		auto nbt_fun = nbt::compound(res);
		CHECK( std_get.size() == 0 );
		CHECK( nbt_fun.size() == 0 );
	}
	SECTION( "Notch's `Hello World' example in explicit form" )
	{
		auto iss = make_stream
(	"\x0A"
		"\x0A"
		"\x00\x0B"
		"\x68\x65\x6C\x6C\x6F\x20\x77\x6F\x72\x6C\x64"
			"\x08"
			"\x00\x04"
			"\x6E\x61\x6D\x65"
				"\x00\x09"
				"\x42\x61\x6E\x61\x6E\x72\x61\x6D\x61"
		"\x00"
	"\x00"
,	35
);
		auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
		auto nbt = nbt::compound(res);
		CHECK( nbt.size() == 1 );
		CHECK( nbt.begin()->first == "hello world" );
		CHECK( nbt::compound(nbt["hello world"]).size() == 1 );
		CHECK
		(	nbt::compound(nbt["hello world"]).begin()->first
			== "name"
		);
		CHECK
		(	nbt::string
			(	nbt::compound(nbt["hello world"])["name"]
			) == "Bananrama"
		);
	}
	SECTION( R"(TAG_Compound as {{"vivalalgerie", {1, 2, 3}}, {"Shrek movies", {2001, 2004, 2007, 2010, 2019}}})" )
	{
		auto iss = make_stream
(	"\x0A"
		"\x09"
		"\x00\x0C"
		"\x76\x69\x76\x61\x6C\x61\x6C\x67\x65\x72\x69\x65"
		"\x03"
		"\x00\x00\x00\x03"
			"\x00\x00\x00\x01"
			"\x00\x00\x00\x02"
			"\x00\x00\x00\x03"
		"\x09"
		"\x00\x0C"
		"\x53\x68\x72\x65\x6B\x20\x6D\x6F\x76\x69\x65\x73"
		"\x03"
		"\x00\x00\x00\x05"
			"\x00\x00\x07\xD1"
			"\x00\x00\x07\xD4"
			"\x00\x00\x07\xD7"
			"\x00\x00\x07\xDA"
			"\x00\x00\x07\xE3"
	"\x00"
,	74
);
		auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
		auto nbt = nbt::compound(res);
		CHECK( nbt.size() == 2 );
		CHECK( nbt::list.as<nbt::int_>(nbt["vivalalgerie"])[0] == 1 );
		CHECK( nbt::list.as<nbt::int_>(nbt["vivalalgerie"])[1] == 2 );
		CHECK( nbt::list.as<nbt::int_>(nbt["vivalalgerie"])[2] == 3 );
		CHECK( nbt::list.as<nbt::int_>(nbt["Shrek movies"])[0] == 2001);
		CHECK( nbt::list.as<nbt::int_>(nbt["Shrek movies"])[1] == 2004);
		CHECK( nbt::list.as<nbt::int_>(nbt["Shrek movies"])[2] == 2007);
		CHECK( nbt::list.as<nbt::int_>(nbt["Shrek movies"])[3] == 2010);
		CHECK( nbt::list.as<nbt::int_>(nbt["Shrek movies"])[4] == 2019);
	}
}

TEST_CASE( "parsing TAG_Int_Array" )
{
	SECTION( "valid TAG_Int_Array" )
	{
		using cont = std::vector<std::int_least32_t>;
		std::array tests =
		{	std::tuple
			(	"\x0B""\x00\x00\x00\x00", 5
			,	cont{}
			)
		,	std::tuple
			(	"\x0B""\xFF\xFF\xFF\xFF", 5
			,	cont{}
			)
		,	std::tuple
			(	"\x0B""\x00\x00\x00\x04"
					"\x00\x0F\xFF\xFF"
					"\x00\x1F\xFF\xFF"
					"\x00\x3F\xFF\xFF"
					"\x00\x7F\xFF\xFF"
			,	21
			,	cont{1048575, 2097151, 4194303, 8388607}
			)
		};
		for (auto const &[s, len, expected] : tests)
		{
			auto iss = make_stream(s, len);
			auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
			auto std_get = std::get<nbt::int_array>(*res);
			auto nbt_fun = nbt::int_array(res);
			CHECK( std_get == nbt_fun );
			CHECK( std_get == expected );
			CHECK( nbt_fun == expected );
		}
	}
}

TEST_CASE( "parsing TAG_Long_Array" )
{
	SECTION( "valid TAG_Long_Array" )
	{
		using cont = std::vector<std::int_least64_t>;
		std::array tests =
		{	std::tuple
			(	"\x0C""\x00\x00\x00\x00", 5
			,	cont{}
			)
		,	std::tuple
			(	"\x0C""\xFF\xFF\xFF\xFF", 5
			,	cont{}
			)
		,	std::tuple
			(	"\x0C""\x00\x00\x00\x08"
					"\x00\x00\x00\x00\x00\x00\x00\x2A"
					"\x00\x00\x00\x00\x00\x00\x03\xC0"
					"\x00\x00\x00\x00\x00\xC6\xC6\xB8"
					"\x00\x00\x00\x00\x32\x97\xB9\xBC"
					"\x00\x00\x00\x94\xBC\x54\x5F\xE0"
					"\x00\x00\x07\x75\x4C\x94\x1F\x6B"
					"\x00\x4E\x9F\x4C\xEE\xAE\xE2\x4E"
					"\xA3\xF1\x39\xE3\x8F\x77\x5C\x5A"
			,	69
			,	cont
				{	42L
				,	960L
				,	13027000L
				,	848804284L
				,	638814805984L
				,	8200377343851L
				,	22130200954200654L
				,	-6633457126612706214L
				}
			)
		};
		for (auto const &[s, len, expected] : tests)
		{
			auto iss = make_stream(s, len);
			auto res = nbt::parse<nbt::parsing::no_implicit>(iss);
			auto std_get = std::get<nbt::long_array>(*res);
			auto nbt_fun = nbt::long_array(res);
			CHECK( std_get == nbt_fun );
			CHECK( std_get == expected );
			CHECK( nbt_fun == expected );
		}
	}
}
