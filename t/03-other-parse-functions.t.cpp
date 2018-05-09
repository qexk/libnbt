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

#include <cstring>

using namespace nbt::unqualified;

TEST_CASE( "other parse functions" )
{
	char const bytes[] = "\x0A\x00\x0B\x68\x65\x6C\x6C\x6F\x20\x77\x6F\x72\x6C\x64\x08\x00\x04\x6E\x61\x6D\x65\x00\x09\x42\x61\x6E\x61\x6E\x72\x61\x6D\x61\x00";

	SECTION( "hello_world.nbt is correct" )
	{
		std::ifstream ifs("t/hello_world.nbt", std::ios::binary);
		char file[sizeof(bytes) - 1];
		ifs.read(file, sizeof(bytes) - 1);
		REQUIRE( std::memcmp(bytes, file, sizeof(bytes) - 1) == 0 );
	}

	SECTION( "parse_str and parse_file" )
	{
		auto from_str = nbt::parse_str
		(	std::string(bytes, sizeof(bytes) - 1)
		);
		auto from_file = nbt::parse_file("t/hello_world.nbt");
		CHECK
		(	string
			(	compound
				(	compound(from_str)["hello world"]
				)["name"]
			) == string
			(	compound
				(	compound(from_file)["hello world"]
				)["name"]
			)
		);
	}
}
