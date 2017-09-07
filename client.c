# include "qdb.h"
int main(void) {
	struct qdb _qdb;
	in_addr_t addr;
	addr = inet_addr("127.0.0.1");
	qdb_connect(&_qdb, addr);
}
