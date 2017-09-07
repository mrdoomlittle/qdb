INC=-Imdlerr/inc
all:
	gcc -c -std=c11 $(INC) -o qdb.o qdb.c
	gcc -std=c11 $(INC) -o server server.c qdb.o -lpthread
	gcc -std=c11 $(INC) -o client client.c qdb.o -lpthread
