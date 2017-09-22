# include "qdb.h"
# include <string.h>
int main() {
	struct qdb _qdb;
	qdb_init(&_qdb);

	char *id = "Hello";
	mdl_uint_t id_bc = strlen(id);
	mdl_u32_t pass = 21;
	_qdb_add_user(id, id_bc, pass);



	user_t *user = _qdb_find_user(id, id_bc);
	printf("name: %s, pass: %u\n", user->id, user->passwd);
	qdb_de_init(&_qdb);
}
