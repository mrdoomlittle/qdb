SHELL :=/bin/bash
DEF_INSTALL=/usr/local
DESTDIR=$(DEF_INSTALL)

NO_BINARY=false
INC_DIR_NAME=include

INTLEN_INC=$(DEF_INSTALL)/$(INC_DIR_NAME)
INTLEN_LIB=$(DEF_INSTALL)/lib
EINT_T_INC=$(DEF_INSTALL)/$(INC_DIR_NAME)

INC=-Iinc -I$(INTLEN_INC) -I$(EINT_T_INC)
LIB=-Llib -L$(INTLEN_LIB)
LL=-lgetdigit -lintlen

ARC=
R_ARC=
CFG=
RUST_LIBS=false

all: build

ARC64:
	make build ARC=-DARC64 R_ARC=ARC64 CFG=--cfg RUST_LIBS=$(RUST_LIBS)\
	INTLEN_INC=$(INTLEN_INC) INTLEN_LIB=$(INTLEN_LIB) EINT_T_INC=$(EINT_T_INC)
ARC32:
	make build ARC=-DARC32 R_ARC=ARC32 CFG=--cfg RUST_LIBS=$(RUST_LIBS)\
	INTLEN_INC=$(INTLEN_INC) INTLEN_LIB=$(INTLEN_LIB) EINT_T_INC=$(EINT_T_INC)
build: src/getdigit.o src/libgetdigit.a
	cp src/getdigit.hpp inc
	cp src/libgetdigit.a lib
	
	if [ $(RUST_LIBS) = true ]; then\
		make rust-libs R_ARC=$(R_ARC) CFG=$(CFG);\
		cp src/libgetdigit.rlib rlib;\
		rustc -Llib -Lrlib -o bin/getdigit.rust getdigit.rs -lgetdigit;\
	fi;
	if [ $(NO_BINARY) = false ]; then\
		g++ -Wall -std=c++11 $(INC) $(LIB) $(ARC) -o bin/getdigit getdigit.cpp $(LL);\
	fi;
rust-libs: src/libgetdigit.rlib
	
src/libgetdigit.a: src/getdigit.o
	ar rcs src/libgetdigit.a src/getdigit.o

src/getdigit.o: src/getdigit.cpp
	g++ -c -Wall -fPIC -std=c++11 $(INC) $(ARC) -o src/getdigit.o src/getdigit.cpp

src/libgetdigit.rlib: src/getdigit.rs
	rustc -L/usr/local/lib $(CFG) $(R_ARC) --crate-type=lib -o src/libgetdigit.rlib src/getdigit.rs -lintlen -lstdc++

clean:
	rm -f bin/*
	rm -f lib/*.a
	rm -f rlib/*.rlib
	rm -f inc/*.hpp
	rm -f src/*.o
	rm -f src/*.a
	rm -f src/*.rlib
install:
	mkdir -p $(DESTDIR)/bin
	mkdir -p $(DESTDIR)/lib
	mkdir -p $(DESTDIR)/rlib
	mkdir -p $(DESTDIR)/$(INC_NAME)
	cp bin/getdigit $(DESTDIR)/bin/getdigit
	cp lib/libgetdigit.a $(DESTDIR)/lib/libgetdigit.a
	if [ -f rlib/libgetdigit.rlib ]; then \
		cp rlib/libgetdigit.rlib $(DESTDIR)/rlib/libgetdigit.rlib;\
	fi;
	cp inc/getdigit.hpp $(DESTDIR)/$(INC_NAME)/getdigit.hpp
uninstall:
	rm -f $(DESTDIR)/bin/getdigit
	rm -f $(DESTDIR)/lib/libgetdigit.a
	rm -f $(DESTDIR)/rlib/libgetdigit.rlib
	rm -f $(DESTDIR)/$(INC_NAME)/getdigit.hpp
