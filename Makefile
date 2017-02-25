CURR_PATH=${CURDIR}
CXXFLAGS=-Iechar_t/inc -Ieint_t/inc -Iintlen/inc -Igetdigit/inc -Ito_string/inc -Iserializer/inc -Lintlen/lib -Lgetdigit/lib -Lto_string/lib
LDFLAGS=-lboost_system -lboost_filesystem -lpthread -lboost_thread -lto_string -lgetdigit -lintlen
ARC=-DARC64
all: qgdb_server qgdb_client

addition:
	cd intlen; make ARC64 EINT_T_INC=$(CURR_PATH)/eint_t/inc; cd ../;
	cd getdigit; make ARC64 EINT_T_INC=$(CURR_PATH)/eint_t/inc INTLEN_INC=$(CURR_PATH)/intlen/inc INTLEN_LIB=$(CURR_PATH)/intlen/lib; cd ../;
	cd to_string; make ECHAR_T=$(CURR_PATH)/echar_t/inc EINT_T_INC=$(CURR_PATH)/eint_t/inc GETDIGIT_INC=$(CURR_PATH)/getdigit/inc INTLEN_INC=$(CURR_PATH)/intlen/inc GETDIGIT_LIB=$(CURR_PATH)/getdigit/lib INTLEN_LIB=$(CURR_PATH)/intlen/lib; cd ../;

src/qgdb_server.o: src/qgdb_server.cpp
	g++ -c -std=c++11 $(ARC) $(CXXFLAGS) -o src/qgdb_server.o src/qgdb_server.cpp

src/conn_handler.o: src/conn_handler.cpp
	g++ -c -std=c++11 $(ARC) $(CXXFLAGS) -o src/conn_handler.o src/conn_handler.cpp

qgdb_server: src/qgdb_server.o src/conn_handler.o addition
	g++ -std=c++11 $(ARC) $(CXXFLAGS) -o qgdb_server.exec src/qgdb_server.o src/conn_handler.o $(LDFLAGS)

src/qgdb_client.o: src/qgdb_client.cpp
	g++ -c -std=c++11 $(ARC) $(CXXFLAGS) -o src/qgdb_client.o src/qgdb_client.cpp

qgdb_client: src/qgdb_client.o addition
	g++ -std=c++11 $(ARC) $(CXXFLAGS) -o qgdb_client.exec src/qgdb_client.o $(LDFLAGS)

clean:
	cd intlen; make clean; cd ../;
	cd getdigit; make clean; cd ../;
	cd to_string; make clean; cd ../;
	rm src/*.o
	rm *.exec
