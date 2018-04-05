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
CURL			:=curl
RM			:=rm -rf --
PROVE			:=prove

VENDOR_DIR		:=vendor/

define CATCH_URL
https://raw.githubusercontent.com/catchorg/Catch2/master/single_include/catch.hpp
endef
CATCH_HPP		:=$(VENDOR_DIR)catchorg/Catch2/catch.hpp

CXX			:=g++ -std=c++1z
CPPFLAGS		:=-I ${dir $(CATCH_HPP)}

SRCS			:=

.SUFFIXES:
.SUFFIXES: .o .cpp

%.t.o: $(CATCH_HPP)
%.t:: %.t.o

.PHONY: all
all: tests

.PHONY: tests
tests: $(CATCH_HPP) $(SRCS:.cpp=)

$(CATCH_HPP):
	$(CURL) '$(CATCH_URL)' -o $@ --create-dirs

.PHONY: clean
clean:
	$(RM) $(SRCS:.cpp=)

.PHONY: distclean
distclean: clean
	$(RM) $(VENDOR_DIR)

.PHONY: check
check:
	@$(PROVE)
