# include "qdb.h"
# include <string.h>
# include <malloc.h>
extern mdl_u32_t qdb_hash(mdl_u8_t const*, mdl_uint_t);
extern void* qdb_mem_dupe(void*, mdl_uint_t);
void **users;

# define PAGE_SIZE 13
struct blk {
	mdl_u32_t val;
	mdl_u8_t *key;
	mdl_uint_t bc;
	void *data;
};

mdl_uint_t static page_c = 0;
user_t* _qdb_find_user(mdl_u8_t const*__id, mdl_u8_t const __id_bc) {
	mdl_u32_t val = qdb_hash(__id, __id_bc);
	void *p = *(users+(val&0xFF));
	if (!p) return NULL;

	mdl_uint_t size = *(mdl_uint_t*)p;
	p = (void*)((mdl_u8_t*)p+sizeof(mdl_uint_t));

	struct blk *itr = (struct blk*)p;
	while(itr != (struct blk*)((mdl_u8_t*)p+(size*sizeof(struct blk)))) {
		if (itr->val == val && itr->bc == __id_bc)
			if (!memcmp(itr->key, __id, __id_bc)) return (user_t*)itr->data;
	}

	return NULL;
}

mdl_err_t _qdb_add_user(mdl_u8_t const *__id, mdl_u8_t const __id_bc, mdl_u32_t __passwd) {
	mdl_u32_t val = qdb_hash(__id, __id_bc);
	void **p = users+(val&0xFF);
	if (!*p)
		*p = malloc(sizeof(mdl_uint_t)+(((++page_c)*PAGE_SIZE)*sizeof(struct blk)));
	else
		*p = realloc(*p, sizeof(mdl_uint_t)+(((++page_c)*PAGE_SIZE)*sizeof(struct blk)));
	struct blk *b = (struct blk*)(((mdl_u8_t*)*p)+sizeof(mdl_uint_t)+(((*(mdl_uint_t*)*p)++)*sizeof(struct blk)));

	*b = (struct blk) {
		.val = val,
		.key = qdb_mem_dupe((mdl_u8_t*)__id, __id_bc),
		.bc = __id_bc,
		.data = malloc(sizeof(user_t))
	};

	user_t *user = (user_t*)b->data;
	user->id = qdb_mem_dupe((mdl_u8_t*)__id, __id_bc);
	user->id_bc = __id_bc;
	user->passwd = __passwd;
	return MDL_SUCCESS;
}

mdl_err_t _qdb_del_user(mdl_u8_t const *__id, mdl_u8_t const __id_bc) {
	mdl_u32_t val = qdb_hash(__id, __id_bc);
	void **p = users+(val&0xFF);
	if (!*p) return MDL_FAILURE;

	mdl_uint_t size;
	if ((size = *(mdl_uint_t*)*p) == 1) {
		free(*p);
		*p = NULL;
		return MDL_SUCCESS;
	}

	struct blk *b = (struct blk*)(((mdl_u8_t*)*p)+sizeof(mdl_uint_t)+(((*(mdl_uint_t*)*p)++)*sizeof(struct blk)));
	free(b->key);
	free(((user_t*)b->data)->id);
	free(b->data);

	struct blk *last = (struct blk*)(((mdl_u8_t*)*p)+sizeof(mdl_uint_t)+(((++page_c)*PAGE_SIZE)*sizeof(struct blk)));
	last = (struct blk*)((mdl_u8_t*)last-sizeof(struct blk));

	memcpy(b, last, sizeof(struct blk));
	(*(mdl_uint_t*)p)--;
	return MDL_SUCCESS;
}

