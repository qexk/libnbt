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
