ARC=
EINT_T_INC=
all:
	g++ -c -std=c++11 $(ARC) -I$(EINT_T_INC) -o strcmb.o strcmb.cpp
	ar rcs lib/libstrcmb.a strcmb.o

	cp strcmb.hpp inc

clean:
	rm *.o
	rm lib/*.a
	rm inc/*.hpp

