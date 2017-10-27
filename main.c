# include "qdb.h"
# include <string.h>
int main() {
	struct qdb _qdb;
	qdb_init(&_qdb);

/*
	_qdb_mem_free(&_qdb, (void*)_qdb_mem_alloc(&_qdb, 30));

	for (;;) {
	printf("alloc.\n");
	mdl_u32_t *b = _qdb_mem_alloc(&_qdb, sizeof(mdl_u32_t));
	mdl_u64_t *a = _qdb_mem_alloc(&_qdb, sizeof(mdl_u64_t));
	*a = 21299;

	printf("free.\n");
	_qdb_mem_free(&_qdb, (void*)a);
	_qdb_mem_free(&_qdb, (void*)b);
	}
*/
	mdl_u32_t *pile_id;
	struct qdb_pile *pile;
	_qdb_creat_pile(&_qdb, &pile_id, &pile);
	printf("id: %u\n", *pile_id);


	mdl_u32_t *record_no;
	mdl_u8_t p = 1;
//	for (;p != 12;p++) {
	_qdb_add_record(&_qdb, pile, &record_no, p*10);
/*
	mdl_uint_t o = 'a', i = 0;
	for (;i < p*10;i++, o++) {
		qdb_set_record(&_qdb, pile, &o, i, 1, *record_no);
	}
*/
//	qdb_rm_record(&_qdb, pile, record_no);//
//	}
//	_qdb_mem_update(&_qdb, x);
	qdb_de_init(&_qdb);
}
