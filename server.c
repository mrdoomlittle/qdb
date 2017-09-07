# include "qdb.h"
int main(void) {
	struct qdb _qdb;
	qdb_init(&_qdb);
	qdb_begin(&_qdb);
	qdb_de_init(&_qdb);
}
