INC=-Imdlerr/inc
OBJS=user.o qdb.o m.o
all: clean
	gcc -c -std=gnu11 $(INC) -o m.o m.c
	gcc -c -std=gnu11 $(INC) -o user.o user.c
	gcc -c -std=gnu11 $(INC) -o qdb.o qdb.c
	gcc -std=gnu11 $(INC) -o main main.c $(OBJS) -lpthread
	gcc -std=gnu11 $(INC) -o server server.c $(OBJS) -lpthread
	gcc -std=gnu11 $(INC) -o client client.c $(OBJS) -lpthread
clean:
	rm -f *.o
