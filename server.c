# include "qdb.h"
# include <string.h>
extern mdl_u32_t qdb_hash(mdl_u8_t const*, mdl_uint_t);
int main(void) {
	struct qdb _qdb;
	qdb_init(&_qdb);

	char *id = "id";
	char *passwd_s = "passwd";
	mdl_u32_t passwd = qdb_hash(passwd_s, strlen(passwd_s));
	_qdb_add_user((mdl_u8_t*)id, strlen(id), passwd);

	qdb_begin(&_qdb);
	qdb_de_init(&_qdb);
}
