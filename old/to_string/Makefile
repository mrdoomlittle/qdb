SHELL :=/bin/bash
DEF_INSTALL=/usr/local
DESTDIR=$(DEF_INSTALL)

NO_BINARY=false
INC_DIR_NAME=include

GETDIGIT_INC=$(DEF_INSTALL)/$(INC_DIR_NAME)
GETDIGIT_LIB=$(DEF_INSTALL)/lib
INTLEN_INC=$(DEF_INSTALL)/$(INC_DIR_NAME)
INTLEN_LIB=$(DEF_INSTALL)/lib
ECHAR_T_INC=$(DEF_INSTALL)/$(INC_DIR_NAME)
EINT_T_INC=$(DEF_INSTALL)/$(INC_DIR_NAME)

INC=-Iinc -I$(ECHAR_T_INC) -I$(EINT_T_INC) -I$(GETDIGIT_INC) -I$(INTLEN_INC)
LIB=-Llib -L$(GETDIGIT_LIB) -L$(INTLEN_LIB)
ARC=-DARC64
LL=-lto_string -lgetdigit -lintlen
all:
	g++ -c -std=c++11 $(INC) $(ARC) -o src/to_string.o src/to_string.cpp
	ar rcs lib/libto_string.a src/to_string.o

	cp src/to_string.hpp inc/to_string.hpp

	if [ $(NO_BINARY) = false ]; then\
		g++ -std=c++11 $(INC) $(LIB) $(ARC) -o bin/to_string to_string.cpp $(LL);\
	fi;

install:
	cp lib/libto_string.a $(DESTDIR)/lib
	cp src/to_string.hpp $(DESTDIR)/$(INC_DIR_NAME)
uninstall:
	rm -f $(DESTDIR)/lib/libto_string.a
	rm -f $(DESTDIR)/$(INC_DIR_NAME)/to_string.hpp
clean:
	rm -f src/*.o
	rm -f lib/*.a
	rm -f inc/*.hpp
	rm -f bin/*

