CURR_PATH=${CURDIR}
CXXFLAGS=-I$(CURR_PATH)/eint_t/inc
LDFLAGS=-lboost_system -lboost_filesystem -lpthread -lboost_thread
ARC=-DARC64
all: qgdb_server

src/qgdb_server.o: src/qgdb_server.cpp
	g++ -c -std=c++11 $(ARC) $(CXXFLAGS) -o src/qgdb_server.o src/qgdb_server.cpp

src/conn_handler.o: src/conn_handler.cpp
	g++ -c -std=c++11 $(ARC) $(CXXFLAGS) -o src/conn_handler.o src/conn_handler.cpp

qgdb_server: src/qgdb_server.o src/conn_handler.o
	g++ -std=c++11 $(ARC) $(CXXFLAGS) -o qgdb_server.exec src/qgdb_server.o src/conn_handler.o $(LDFLAGS)

clean:
	rm src/*.o
	rm *.exec
