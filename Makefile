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
CATCH_HPPS		:=${addprefix $(CATCH_DIR), \
			  catch.hpp \
			  catch_reporter_tap.hpp \
			  }

CXX			:=g++ -std=c++1z
CPPFLAGS		+=-I $(CATCH_DIR) -I $(INCLUDE_DIR)
CXXFLAGS		:=-W -Wall -Werror -pedantic -Wno-multichar
LDLIBS			:=-lboost_iostreams

SRCS			:=t/00-utilities/00-list_wrapper.t.cpp \
			  t/00-utilities/01-compound_wrapper.t.cpp \
			  t/01-parsing-no_implicit.t.cpp \
			  t/02-parsing-implicit.t.cpp \
			  t/03-other-parse-functions.t.cpp

.SUFFIXES:
.SUFFIXES: .cpp .o

.PHONY: all
all: tests

.PHONY: tests
tests: $(SRCS:.cpp=)

$(CATCH_DIR)%.hpp:
	$(CURL) '$(CATCH_URL)$*.hpp' -o $@ --create-dirs

$(SRCS): | $(CATCH_HPPS)

%.t: %.t.o t/main.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

.PHONY: clean
clean:
	$(RM) $(SRCS:.cpp=) $(SRCS:.cpp=.o)

.PHONY: distclean
distclean: clean
	$(RM) $(VENDOR_DIR) t/main.o

.PHONY: check
check: tests
	@$(PROVE)

.PRECIOUS: %.o
