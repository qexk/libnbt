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

#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_FAST_COMPILE
#define CATCH_CONFIG_DISABLE_MATCHERS
#define CATCH_CONFIG_DEFAULT_REPORTER "tap"
#include "catch.hpp"
#include "catch_reporter_tap.hpp"

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
