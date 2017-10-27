# include "qdb.h"
# include <malloc.h>
# include <fcntl.h>
struct qdb_pile **piles = NULL;
mdl_uint_t no_piles = 0;
mdl_uint_t f_off = 0;
mdl_u32_t last_blk = 0;

mdl_u32_t lgtfree = 0;
mdl_u32_t *free_blks = NULL;
mdl_uint_t no_free_blks = 0;
mdl_err_t _qdb_creat_pile(struct qdb *__qdb, mdl_u32_t **__id, struct qdb_pile **__pile) {
	if (!piles) {
		piles = (struct qdb_pile**)malloc(sizeof(struct qdb_pile*));
		no_piles++;
	} else {
		piles = (struct qdb_pile**)realloc(piles, (++no_piles)*sizeof(struct qdb_pile*));
	}

	struct qdb_pile *pile = *(piles+(no_piles-1)) = (struct qdb_pile*)malloc(sizeof(struct qdb_pile));
	*pile = (struct qdb_pile) {
		.records = NULL,
		.no_records = 0,
		.id = no_piles-1
	};

	*__id = &pile->id;
	if (__pile != NULL)
		*__pile = pile;
}

struct qdb_pile* qdb_get_pile(mdl_u32_t __id) {
	return *(piles+__id);
}

mdl_err_t _qdb_del_pile(struct qdb *__qdb, mdl_u32_t __id) {
	struct qdb_pile *pile = *(piles+__id);

	struct qdb_record **itr = pile->records;
	while(itr != pile->records+pile->no_records) {
		struct qdb_record *record = *itr;
		_qdb_rm_record(__qdb, pile, record->no);
		itr++;
	}
}

mdl_err_t _qdb_add_record(struct qdb *__qdb, struct qdb_pile *__pile, mdl_u32_t **__no, mdl_uint_t __size) {
	if (!__pile->records) {
		__pile->records = (struct qdb_record**)malloc(sizeof(struct qdb_record*));
		__pile->no_records++;
	} else {
		__pile->records = (struct qdb_record**)realloc(__pile->records, (++__pile->no_records)*sizeof(struct qdb_record*));
	}

	struct qdb_record *record = *(__pile->records+(__pile->no_records-1)) = (struct qdb_record*)malloc(sizeof(struct qdb_record));
	*record = (struct qdb_record) {
		.no = __pile->no_records-1,
		.size = __size
	};

	*__no = &record->no;

	if (no_free_blks > 0 && __size <= lgtfree) {
		mdl_u32_t *itr = free_blks;
		for(;itr != free_blks+no_free_blks;itr++) {
			struct blkd blk;
			lseek(__qdb->fd, (*itr)-sizeof(struct blkd), SEEK_SET);
			read(__qdb->fd, (void*)&blk, sizeof(struct blkd));
			if (blk.size >= __size) {
				record->f_off = *itr;
				return 0;
			}
		}
	}

	record->f_off = f_off+sizeof(struct blkd);

	lseek(__qdb->fd, f_off, SEEK_SET);
	struct blkd _blkd = {
		.size = __size,
		.is_free = 0,
		.next = last_blk
	};

	last_blk = ((record->f_off)<<1)|1;

	write(__qdb->fd, &_blkd, sizeof(struct blkd));
	posix_fallocate(__qdb->fd, record->f_off, __size);
	f_off+=__size+sizeof(struct blkd);
}

mdl_err_t _qdb_resize_record(struct qdb *__qdb, struct qdb_pile *__pile, mdl_u32_t __no, mdl_uint_t __size) {
	
}

mdl_err_t _qdb_set_record(struct qdb *__qdb, struct qdb_pile *__pile, void *__buff, mdl_uint_t __off, mdl_uint_t __bc, mdl_u32_t __no) {
	struct qdb_record *record = *(__pile->records+__no);
	lseek(__qdb->fd, record->f_off+__off, SEEK_SET);
	write(__qdb->fd, __buff, __bc);
}

mdl_err_t _qdb_get_record(struct qdb *__qdb, struct qdb_pile *__pile, void *__buff, mdl_uint_t __off, mdl_uint_t __bc, mdl_u32_t __no) {
	struct qdb_record *record = *(__pile->records+__no);
	lseek(__qdb->fd, record->f_off+__off, SEEK_SET);
	read(__qdb->fd, __buff, __bc);
}

mdl_err_t _qdb_rm_record(struct qdb *__qdb, struct qdb_pile *__pile, mdl_u32_t __no) {
	struct qdb_record *record = *(__pile->records+__no);
	if (f_off == record->f_off+record->size) {
		printf("del, %u\n", __no);
		f_off = record->f_off-sizeof(struct blkd);
		goto _sk_soft;
	}

	if (!no_free_blks) {
		free_blks = (mdl_u32_t*)malloc(sizeof(mdl_u32_t));
		no_free_blks++;
	} else {
		free_blks = (mdl_u32_t*)realloc(free_blks, (++no_free_blks)*sizeof(mdl_u32_t));
	}

	*(free_blks+(no_free_blks-1)) = record->f_off;
	if (record->size > lgtfree) lgtfree = record->size;

	_sk_soft:
	free(record);
	*(__pile->records+__no) = NULL;

	if (!(__pile->no_records-1)) {
		free(__pile->records);
		__pile->no_records--;
		__pile->records = NULL;
	} else {
		*(__pile->records+__no) = *(__pile->records+(__pile->no_records-1));
		(*(__pile->records+__no))->no = __no;
		__pile->records = (struct qdb_record**)realloc(__pile->records, (--__pile->no_records)*sizeof(struct qdb_record*));
	}
}
