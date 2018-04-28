# libnbt++ --- A C++ library for parsing, generating and manipulating NBT
# Copyright (C) 2018 Alexandre Szymocha <alexandre@szymocha.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

SHELL			:=/bin/sh
RM			:=rm -rf --
CURL			:=curl -sS
PROVE			:=prove

VENDOR_DIR		:=vendor/
INCLUDE_DIR		:=include/

CATCH_DIR		:=$(VENDOR_DIR)catchorg/Catch2/
CATCH_URL		:=https://raw.githubusercontent.com/catchorg/Catch2/master/single_include/
CATCH_HPPS		:=${addprefix $(CATCH_DIR),\
			  catch.hpp\
			  catch_reporter_tap.hpp\
			  }

CXX			:=g++ -std=c++1z
CPPFLAGS		+=-I $(CATCH_DIR) -I $(INCLUDE_DIR)
CXXFLAGS		:=-W -Wall -Werror -pedantic -Wno-multichar

SRCS			:=t/00-utilities.t.cpp

.SUFFIXES:
.SUFFIXES: .cpp

.PHONY: all
all: tests

.PHONY: tests
tests: $(SRCS:.cpp=)

%.d:: %.t.cpp
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM $< -MT $*.t.cpp \
		| sed -E 's/\S+\.cpp//2' > $@

.PRECIOUS: $(SRCS:.t.cpp=.d)
sinclude $(SRCS:.t.cpp=.d)

$(CATCH_DIR)%.hpp:
	$(CURL) '$(CATCH_URL)$*.hpp' -o $@ --create-dirs

$(SRCS:.cpp=): | $(CATCH_HPPS)

.PHONY: clean
clean:
	$(RM) $(SRCS:.cpp=) $(SRCS:.t.cpp=.d)

.PHONY: distclean
distclean: clean
	$(RM) $(VENDOR_DIR)

.PHONY: check
check: tests
	@$(PROVE)
