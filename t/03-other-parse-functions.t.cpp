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

	SECTION( "t/hello_world.nbt is correct" )
	{
		std::ifstream ifs("t/hello_world.nbt", std::ios::binary);
		char file[sizeof(bytes) - 1];
		ifs.read(file, sizeof(bytes) - 1);
		REQUIRE( ifs.good() );
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

TEST_CASE( "parse_auto with gzipped file" )
{
	char const bytes[] = "\x1F\x8B\x08\x00\x00\x00\x00\x00\x00\x00\xED\x54\xCF\x4F\x1A\x41\x14\x7E\xC2\x02\xCB\x96\x82\xB1\xC4\x10\x63\xCC\xAB\xB5\x84\xA5\xDB\xCD\x42\x11\x89\xB1\x88\x16\x2C\x9A\x0D\x1A\xD8\xA8\x31\x86\xB8\x2B\xC3\x82\x2E\xBB\x66\x77\xB0\xF1\xD4\x4B\x7B\x6C\x7A\xEB\x3F\xD3\x23\x7F\x43\xCF\xBD\xF6\xBF\xA0\xC3\x2F\x7B\x69\xCF\xBD\xF0\x32\xC9\xF7\xE6\xBD\x6F\xE6\x7B\x6F\x26\x79\x02\x04\x54\x72\x4F\x2C\x0E\x78\xCB\xB1\x4D\x8D\x78\xF4\xE3\x70\x62\x3E\x08\x7B\x1D\xC7\xA5\x93\x18\x0F\x82\x47\xDD\xEE\x84\x02\x62\xB5\xA2\xAA\xC7\x78\x76\x5C\x57\xCB\xA8\x55\x0F\x1B\xC8\xD6\x1E\x6A\x95\x86\x86\x0D\xAD\x7E\x58\x7B\x8F\x83\xCF\x83\x4F\x83\x6F\xCF\x03\x10\x6E\x5B\x8E\x3E\xBE\xA5\x38\x4C\x64\xFD\x10\xEA\xDA\x74\xA6\x23\x40\xDC\x66\x2E\x69\xE1\xB5\xD3\xBB\x73\xFA\x76\x0B\x29\xDB\x0B\xE0\xEF\xE8\x3D\x1E\x38\x5B\xEF\x11\x08\x56\xF5\xDE\x5D\xDF\x0B\x40\xE0\x5E\xB7\xFA\x64\xB7\x04\x00\x8C\x41\x4C\x73\xC6\x08\x55\x4C\xD3\x20\x2E\x7D\xA4\xC0\xC8\xC2\x10\xB3\xBA\xDE\x58\x0B\x53\xA3\xEE\x44\x8E\x45\x03\x30\xB1\x27\x53\x8C\x4C\xF1\xE9\x14\xA3\x53\x8C\x85\xE1\xD9\x9F\xE3\xB3\xF2\x44\x81\xA5\x7C\x33\xDD\xD8\xBB\xC7\xAA\x75\x13\x5F\x28\x1C\x08\xD7\x2E\xD1\x59\x3F\xAF\x1D\x1B\x60\x21\x59\xDF\xFA\xF1\x05\xFE\xC1\xCE\xFC\x9D\xBD\x00\xBC\xF1\x40\xC9\xF8\x85\x42\x40\x46\xFE\x9E\xEB\xEA\x0F\x93\x3A\x68\x87\x60\xBB\xEB\x32\x37\xA3\x28\x0A\x8E\xBB\xF5\xD0\x69\x63\xCA\x4E\xDB\xE9\xEC\xE6\xE6\x2B\x3B\xBD\x25\xBE\x64\x49\x09\x3D\xAA\xBB\x94\xFD\x18\x7E\xE8\xD2\x0E\xDA\x6F\x15\x4C\xB1\x68\x3E\x2B\xE1\x9B\x9C\x84\x99\xBC\x84\x05\x09\x65\x59\x16\x45\x00\xFF\x2F\x28\xAE\x2F\xF2\xC2\xB2\xA4\x2E\x1D\x20\x77\x5A\x3B\xB9\x8C\xCA\xE7\x29\xDF\x51\x41\xC9\x16\xB5\xC5\x6D\xA1\x2A\xAD\x2C\xC5\x31\x7F\xBA\x7A\x92\x8E\x5E\x9D\x5F\xF8\x12\x05\x23\x1B\xD1\xF6\xB7\x77\xAA\xCD\x95\x72\xBC\x9E\xDF\x58\x5D\x4B\x97\xAE\x92\x17\xB9\x44\xD0\x80\xC8\xFA\x3E\xBF\xB3\xDC\x54\xCB\x07\x75\x6E\xA3\xB6\x76\x59\x92\x93\xA9\xDC\x51\x50\x99\x6B\xCC\x35\xE6\x1A\xFF\x57\x23\x08\x42\xCB\xE9\x1B\xD6\x78\xC2\xEC\xFE\xFC\x7A\xFB\x7D\x78\xD3\x84\xDF\xD4\xF2\xA4\xFB\x08\x06\x00\x00";

	SECTION( "t/bigtest.nbt.gz is correct" )
	{
		std::ifstream ifs("t/bigtest.nbt.gz", std::ios::binary);
		char file[sizeof(bytes) - 1];
		ifs.read(file, sizeof(bytes) - 1);
		REQUIRE( ifs.good() );
		REQUIRE( std::memcmp(bytes, file, sizeof(bytes) - 1) == 0 );
	}

	SECTION( "parsing t/bigtest.nbt.gz" )
	{
		auto res = nbt::parse_file("t/bigtest.nbt.gz");
		CHECK( res->index() == compound );
		CHECK( compound(res).begin()->first == "Level" );
		CHECK( compound(res)["Level"]->index() == compound );
		CHECK
		(	compound(compound(res)["Level"]).size()
			== 11
		);
		CHECK
		(	compound(compound(res)["Level"])
				["nested compound test"]->index()
			== compound
		);
		// assume .index() is correct
		CHECK
		(	string
			(
				compound
				(	compound
					(	compound
						(	compound(res)
							["Level"]
						)["nested compound test"]
					)["egg"]
				)["name"]
			) == "Eggbert"
		);
		CHECK
		(	float_
			(
				compound
				(	compound
					(	compound
						(	compound(res)
							["Level"]
						)["nested compound test"]
					)["egg"]
				)["value"]
			) == 0.5
		);
		CHECK
		(	string
			(
				compound
				(	compound
					(	compound
						(	compound(res)
							["Level"]
						)["nested compound test"]
					)["ham"]
				)["name"]
			) == "Hampus"
		);
		CHECK
		(	float_
			(
				compound
				(	compound
					(	compound
						(	compound(res)
							["Level"]
						)["nested compound test"]
					)["ham"]
				)["value"]
			) == 0.75
		);
		// intTest
		CHECK
		(	compound(compound(res)["Level"])
				["intTest"]->index()
			== int_
		);
		CHECK
		(	int_
			(	compound(compound(res)["Level"])
					["intTest"]
			) == 2147483647
		);
		// byteTest
		CHECK
		(	compound(compound(res)["Level"])
				["byteTest"]->index()
			== byte
		);
		CHECK
		(	byte
			(	compound(compound(res)["Level"])
					["byteTest"]
			) == 127
		);
		// stringTest
		CHECK
		(	compound(compound(res)["Level"])
				["stringTest"]->index()
			== string
		);
		CHECK
		(	string
			(	compound(compound(res)["Level"])
					["stringTest"]
			) == "HELLO WORLD THIS IS A TEST STRING \xc3\x85\xc3\x84\xc3\x96!"
		);
		// listTest (long)
		CHECK
		(	compound(compound(res)["Level"])
				["listTest (long)"]->index()
			== list
		);
		CHECK
		(	list
			(	compound(compound(res)["Level"])
					["listTest (long)"]
			).front().index() == long_
		);
		CHECK
		(	list.as<long_>
			(	compound(compound(res)["Level"])
					["listTest (long)"]
			) == std::vector{11L, 12L, 13L, 14L, 15L}
		);
		// doubleTest
		CHECK
		(	compound(compound(res)["Level"])
				["doubleTest"]->index()
			== double_
		);
		CHECK
		(	double_
			(	compound(compound(res)["Level"])
					["doubleTest"]
			) == 0.49312871321823148
		);
		// floatTest
		CHECK
		(	compound(compound(res)["Level"])
				["floatTest"]->index()
			== float_
		);
		CHECK
		(	float_
			(	compound(compound(res)["Level"])
					["floatTest"]
			) == 0.49823147058486938f
		);
		// longTest
		CHECK
		(	compound(compound(res)["Level"])
				["longTest"]->index()
			== long_
		);
		CHECK
		(	long_
			(	compound(compound(res)["Level"])
					["longTest"]
			) == 9223372036854775807L
		);
		// listTest (compound) (assume .index() is correct)
		CHECK
		(	compound(compound(res)["Level"])
				["listTest (compound)"]->index()
			== list
		);
		CHECK
		(	list
			(	compound(compound(res)["Level"])
					["listTest (compound)"]
			).front().index() == compound
		);
		CHECK
		(	long_
			(	list.as<compound>
				(	compound
					(	compound(res)
						["Level"]
					)["listTest (compound)"]
				)[0]["created-on"]
			) == 1264099775885L
		);
		CHECK
		(	string
			(	list.as<compound>
				(	compound
					(	compound(res)
						["Level"]
					)["listTest (compound)"]
				)[0]["name"]
			) == "Compound tag #0"
		);
		CHECK
		(	long_
			(	list.as<compound>
				(	compound
					(	compound(res)
						["Level"]
					)["listTest (compound)"]
				)[1]["created-on"]
			) == 1264099775885L
		);
		CHECK
		(	string
			(	list.as<compound>
				(	compound
					(	compound(res)
						["Level"]
					)["listTest (compound)"]
				)[1]["name"]
			) == "Compound tag #1"
		);
		// byteArrayTest (the first 1000 values of (n*n*255+n*7)%100, starting with n=0 (0, 62, 34, 16, 8, ...))
		CHECK
		(	compound(compound(res)["Level"])
				["byteArrayTest (the first 1000 values of (n*n*255+n*7)%100, starting with n=0 (0, 62, 34, 16, 8, ...))"]->index()
			== byte_array
		);
		CHECK
		(	byte_array
			(	compound(compound(res)["Level"])
					["byteArrayTest (the first 1000 values of (n*n*255+n*7)%100, starting with n=0 (0, 62, 34, 16, 8, ...))"]
			).size() == 1000
		);
		std::vector<int8_t> byteArrayTest(1000U);
		std::generate
		(	byteArrayTest.begin(), byteArrayTest.end()
		,	[n = -1] (void) mutable -> std::int8_t {
				return ++n, (n * n * 255 + n * 7) % 100;
			}
		);
		CHECK
		(	byte_array
			(	compound(compound(res)["Level"])
					["byteArrayTest (the first 1000 values of (n*n*255+n*7)%100, starting with n=0 (0, 62, 34, 16, 8, ...))"]
			) == byteArrayTest
		);
		// shortTest
		CHECK
		(	compound(compound(res)["Level"])
				["shortTest"]->index()
			== short_
		);
		CHECK
		(	short_
			(	compound(compound(res)["Level"])
					["shortTest"]
			) == 32767
		);
	}
}

TEST_CASE( "parse_auto with zlibbed file" )
{
	char const bytes[] = "\x78\x9C\xED\x54\xCF\x4F\x1A\x41\x14\x7E\xC2\x02\xCB\x96\x82\xB1\xC4\x10\x63\xCC\xAB\xB5\x84\xA5\xDB\xCD\x42\x11\x89\xB1\x88\x16\x2C\x9A\x0D\x1A\xD8\xA8\x31\x86\xB8\x2B\xC3\x82\x2E\xBB\x66\x77\xB0\xF1\xD4\x4B\x7B\x6C\x7A\xEB\x3F\xD3\x23\x7F\x43\xCF\xBD\xF6\xBF\xA0\xC3\x2F\x7B\x69\xCF\xBD\xF0\x32\xC9\xF7\xE6\xBD\x6F\xE6\x7B\x6F\x26\x79\x02\x04\x54\x72\x4F\x2C\x0E\x78\xCB\xB1\x4D\x8D\x78\xF4\xE3\x70\x62\x3E\x08\x7B\x1D\xC7\xA5\x93\x18\x0F\x82\x47\xDD\xEE\x84\x02\x62\xB5\xA2\xAA\xC7\x78\x76\x5C\x57\xCB\xA8\x55\x0F\x1B\xC8\xD6\x1E\x6A\x95\x86\x86\x0D\xAD\x7E\x58\x7B\x8F\x83\xCF\x83\x4F\x83\x6F\xCF\x03\x10\x6E\x5B\x8E\x3E\xBE\xA5\x38\x4C\x64\xFD\x10\xEA\xDA\x74\xA6\x23\x40\xDC\x66\x2E\x69\xE1\xB5\xD3\xBB\x73\xFA\x76\x0B\x29\xDB\x0B\xE0\xEF\xE8\x3D\x1E\x38\x5B\xEF\x11\x08\x56\xF5\xDE\x5D\xDF\x0B\x40\xE0\x5E\xB7\xFA\x64\xB7\x04\x00\x8C\x41\x4C\x73\xC6\x08\x55\x4C\xD3\x20\x2E\x7D\xA4\xC0\xC8\xC2\x10\xB3\xBA\xDE\x58\x0B\x53\xA3\xEE\x44\x8E\x45\x03\x30\xB1\x27\x53\x8C\x4C\xF1\xE9\x14\xA3\x53\x8C\x85\xE1\xD9\x9F\xE3\xB3\xF2\x44\x81\xA5\x7C\x33\xDD\xD8\xBB\xC7\xAA\x75\x13\x5F\x28\x1C\x08\xD7\x2E\xD1\x59\x3F\xAF\x1D\x1B\x60\x21\x59\xDF\xFA\xF1\x05\xFE\xC1\xCE\xFC\x9D\xBD\x00\xBC\xF1\x40\xC9\xF8\x85\x42\x40\x46\xFE\x9E\xEB\xEA\x0F\x93\x3A\x68\x87\x60\xBB\xEB\x32\x37\xA3\x28\x0A\x8E\xBB\xF5\xD0\x69\x63\xCA\x4E\xDB\xE9\xEC\xE6\xE6\x2B\x3B\xBD\x25\xBE\x64\x49\x09\x3D\xAA\xBB\x94\xFD\x18\x7E\xE8\xD2\x0E\xDA\x6F\x15\x4C\xB1\x68\x3E\x2B\xE1\x9B\x9C\x84\x99\xBC\x84\x05\x09\x65\x59\x16\x45\x00\xFF\x2F\x28\xAE\x2F\xF2\xC2\xB2\xA4\x2E\x1D\x20\x77\x5A\x3B\xB9\x8C\xCA\xE7\x29\xDF\x51\x41\xC9\x16\xB5\xC5\x6D\xA1\x2A\xAD\x2C\xC5\x31\x7F\xBA\x7A\x92\x8E\x5E\x9D\x5F\xF8\x12\x05\x23\x1B\xD1\xF6\xB7\x77\xAA\xCD\x95\x72\xBC\x9E\xDF\x58\x5D\x4B\x97\xAE\x92\x17\xB9\x44\xD0\x80\xC8\xFA\x3E\xBF\xB3\xDC\x54\xCB\x07\x75\x6E\xA3\xB6\x76\x59\x92\x93\xA9\xDC\x51\x50\x99\x6B\xCC\x35\xE6\x1A\xFF\x57\x23\x08\x42\xCB\xE9\x1B\xD6\x78\xC2\xEC\xFE\xFC\x7A\xFB\x7D\x78\xD3\x84\xDF\xF6\x84\x58\x4F";

	SECTION( "t/bigtest.nbt.zlib is correct" )
	{
		std::ifstream ifs("t/bigtest.nbt.zlib", std::ios::binary);
		char file[sizeof(bytes) - 1];
		ifs.read(file, sizeof(bytes) - 1);
		REQUIRE( ifs.good() );
		REQUIRE( std::memcmp(bytes, file, sizeof(bytes) - 1) == 0 );
	}

	SECTION( "parsing t/bigtest.nbt.zlib" )
	{
		auto res = nbt::parse_file("t/bigtest.nbt.zlib");
		CHECK( res->index() == compound );
		CHECK( compound(res).begin()->first == "Level" );
		CHECK( compound(res)["Level"]->index() == compound );
		CHECK
		(	compound(compound(res)["Level"]).size()
			== 11
		);
		CHECK
		(	compound(compound(res)["Level"])
				["nested compound test"]->index()
			== compound
		);
		// assume .index() is correct
		CHECK
		(	string
			(
				compound
				(	compound
					(	compound
						(	compound(res)
							["Level"]
						)["nested compound test"]
					)["egg"]
				)["name"]
			) == "Eggbert"
		);
		CHECK
		(	float_
			(
				compound
				(	compound
					(	compound
						(	compound(res)
							["Level"]
						)["nested compound test"]
					)["egg"]
				)["value"]
			) == 0.5
		);
		CHECK
		(	string
			(
				compound
				(	compound
					(	compound
						(	compound(res)
							["Level"]
						)["nested compound test"]
					)["ham"]
				)["name"]
			) == "Hampus"
		);
		CHECK
		(	float_
			(
				compound
				(	compound
					(	compound
						(	compound(res)
							["Level"]
						)["nested compound test"]
					)["ham"]
				)["value"]
			) == 0.75
		);
		// intTest
		CHECK
		(	compound(compound(res)["Level"])
				["intTest"]->index()
			== int_
		);
		CHECK
		(	int_
			(	compound(compound(res)["Level"])
					["intTest"]
			) == 2147483647
		);
		// byteTest
		CHECK
		(	compound(compound(res)["Level"])
				["byteTest"]->index()
			== byte
		);
		CHECK
		(	byte
			(	compound(compound(res)["Level"])
					["byteTest"]
			) == 127
		);
		// stringTest
		CHECK
		(	compound(compound(res)["Level"])
				["stringTest"]->index()
			== string
		);
		CHECK
		(	string
			(	compound(compound(res)["Level"])
					["stringTest"]
			) == "HELLO WORLD THIS IS A TEST STRING \xc3\x85\xc3\x84\xc3\x96!"
		);
		// listTest (long)
		CHECK
		(	compound(compound(res)["Level"])
				["listTest (long)"]->index()
			== list
		);
		CHECK
		(	list
			(	compound(compound(res)["Level"])
					["listTest (long)"]
			).front().index() == long_
		);
		CHECK
		(	list.as<long_>
			(	compound(compound(res)["Level"])
					["listTest (long)"]
			) == std::vector{11L, 12L, 13L, 14L, 15L}
		);
		// doubleTest
		CHECK
		(	compound(compound(res)["Level"])
				["doubleTest"]->index()
			== double_
		);
		CHECK
		(	double_
			(	compound(compound(res)["Level"])
					["doubleTest"]
			) == 0.49312871321823148
		);
		// floatTest
		CHECK
		(	compound(compound(res)["Level"])
				["floatTest"]->index()
			== float_
		);
		CHECK
		(	float_
			(	compound(compound(res)["Level"])
					["floatTest"]
			) == 0.49823147058486938f
		);
		// longTest
		CHECK
		(	compound(compound(res)["Level"])
				["longTest"]->index()
			== long_
		);
		CHECK
		(	long_
			(	compound(compound(res)["Level"])
					["longTest"]
			) == 9223372036854775807L
		);
		// listTest (compound) (assume .index() is correct)
		CHECK
		(	compound(compound(res)["Level"])
				["listTest (compound)"]->index()
			== list
		);
		CHECK
		(	list
			(	compound(compound(res)["Level"])
					["listTest (compound)"]
			).front().index() == compound
		);
		CHECK
		(	long_
			(	list.as<compound>
				(	compound
					(	compound(res)
						["Level"]
					)["listTest (compound)"]
				)[0]["created-on"]
			) == 1264099775885L
		);
		CHECK
		(	string
			(	list.as<compound>
				(	compound
					(	compound(res)
						["Level"]
					)["listTest (compound)"]
				)[0]["name"]
			) == "Compound tag #0"
		);
		CHECK
		(	long_
			(	list.as<compound>
				(	compound
					(	compound(res)
						["Level"]
					)["listTest (compound)"]
				)[1]["created-on"]
			) == 1264099775885L
		);
		CHECK
		(	string
			(	list.as<compound>
				(	compound
					(	compound(res)
						["Level"]
					)["listTest (compound)"]
				)[1]["name"]
			) == "Compound tag #1"
		);
		// byteArrayTest (the first 1000 values of (n*n*255+n*7)%100, starting with n=0 (0, 62, 34, 16, 8, ...))
		CHECK
		(	compound(compound(res)["Level"])
				["byteArrayTest (the first 1000 values of (n*n*255+n*7)%100, starting with n=0 (0, 62, 34, 16, 8, ...))"]->index()
			== byte_array
		);
		CHECK
		(	byte_array
			(	compound(compound(res)["Level"])
					["byteArrayTest (the first 1000 values of (n*n*255+n*7)%100, starting with n=0 (0, 62, 34, 16, 8, ...))"]
			).size() == 1000
		);
		std::vector<int8_t> byteArrayTest(1000U);
		std::generate
		(	byteArrayTest.begin(), byteArrayTest.end()
		,	[n = -1] (void) mutable -> std::int8_t {
				return ++n, (n * n * 255 + n * 7) % 100;
			}
		);
		CHECK
		(	byte_array
			(	compound(compound(res)["Level"])
					["byteArrayTest (the first 1000 values of (n*n*255+n*7)%100, starting with n=0 (0, 62, 34, 16, 8, ...))"]
			) == byteArrayTest
		);
		// shortTest
		CHECK
		(	compound(compound(res)["Level"])
				["shortTest"]->index()
			== short_
		);
		CHECK
		(	short_
			(	compound(compound(res)["Level"])
					["shortTest"]
			) == 32767
		);
	}
}
