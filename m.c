# include "qdb.h"
# include <malloc.h>
struct qdb_mem mem;
mdl_u16_t static last_blk = 0;

void add_uu_blk(mdl_u16_t, mdl_u16_t*);
void rm_uu_blk(mdl_u16_t);

void* _qdb_mem_alloc(struct qdb *__qdb, mdl_uint_t __bc) {
	if (mem.uu_blk_c > 0) {
		struct mem_blkd *blkd;
		mdl_u16_t *itr = mem.uu_blks;
		while(itr != mem.uu_blks+mem.uu_blk_c) {
			blkd = (struct mem_blkd*)((mem.p+*itr)-sizeof(struct mem_blkd));
			if (blkd->bc >= __bc) {
				printf("found unused block.\n");

				if (blkd->bc > __bc) {
				*(struct mem_blkd*)(mem.p+blkd->off+__bc) = (struct mem_blkd) {
					.off = blkd->off+__bc+sizeof(struct mem_blkd),
					.uu_off = blkd->uu_off,
					.above = (blkd->off<<1)|1,
					.below = blkd->below,
					.bc = blkd->bc-__bc,
					.state = MEM_FREE
				};

				blkd->bc = __bc;
				if (blkd->below&0x1)
					((struct mem_blkd*)((mem.p+(blkd->below>>1))-sizeof(struct mem_blkd)))->above = ((blkd->off+__bc+sizeof(struct mem_blkd))<<1)|1;
				blkd->below = ((blkd->off+__bc+sizeof(struct mem_blkd))<<1)|1;

				*itr = blkd->off+__bc+sizeof(struct mem_blkd);
				} else {
					rm_uu_blk(blkd->uu_off);
					printf("del\n");
				}

				blkd->state = MEM_USED;
				return (void*)(mem.p+blkd->off);
			}
			itr++;
		}
	}

	struct mem_blkd *blkd = (struct mem_blkd*)(mem.p+mem.off);
	*blkd = (struct mem_blkd) {
		.off = mem.off+sizeof(struct mem_blkd),
		.uu_off = 0,
		.above = 0, .below = 0,
		.bc = __bc,
		.state = MEM_USED
	};

	if (last_blk&0x1) {
		((struct mem_blkd*)((mem.p+(last_blk>>1)))-sizeof(struct mem_blkd))->below = (blkd->off<<1)|1;
		blkd->above = last_blk;
	}
	last_blk = (blkd->off<<1)|1;
	printf("hi\n");

	mem.off+=__bc+sizeof(struct mem_blkd);
	return (void*)(mem.p+blkd->off);
}

void add_uu_blk(mdl_u16_t __off, mdl_u16_t *__uu_off) {
	printf("add: ------> %u\n", mem.uu_blk_c);
	if (!mem.uu_blk_c) {
		*(mem.uu_blks = (mdl_u16_t*)malloc(sizeof(mdl_u16_t))) = __off;
		mem.uu_blk_c++;
		return;
	}

	mem.uu_blks = (mdl_u16_t*)realloc(mem.uu_blks, (++mem.uu_blk_c)*sizeof(mdl_u16_t));
	*(mem.uu_blks+(mem.uu_blk_c-1)) = __off;
	*__uu_off = mem.uu_blk_c-1;
}

void rm_uu_blk(mdl_u16_t __off) {
	if (__off > mem.uu_blk_c) {
		printf("error.\n");
	}
	if (!(mem.uu_blk_c-1)) {
		free(mem.uu_blks);
		mem.uu_blks = NULL;
		mem.uu_blk_c--;
		return;
	}

	if (__off != mem.uu_blk_c-1)
		*(mem.uu_blks+__off) = *(mem.uu_blks+(mem.uu_blk_c-1));
	mem.uu_blks = (mdl_u16_t*)realloc(mem.uu_blks, (--mem.uu_blk_c)*sizeof(mdl_u16_t));
}

void _qdb_mem_free(struct qdb *__qdb, void *__p) {
	struct mem_blkd *blkd = (struct mem_blkd*)((mdl_u8_t*)__p-sizeof(struct mem_blkd));
	struct mem_blkd *above = NULL, *below = NULL;
	while(blkd->above&0x1) {
		above = (struct mem_blkd*)((mem.p+(blkd->above>>1))-sizeof(struct mem_blkd));
		if (above->state == MEM_USED) break;
		blkd->off = above->off;
		blkd->bc += above->bc+sizeof(struct mem_blkd);
		blkd->above = above->above;
		printf("found free upper block.\n");
		rm_uu_blk(above->uu_off);
	}

	while(blkd->below&0x1) {
		below = (struct mem_blkd*)((mem.p+(blkd->below>>1))-sizeof(struct mem_blkd));
		if (below->state == MEM_USED) break;
		blkd->bc += below->bc+sizeof(struct mem_blkd);
		blkd->below = below->below;
		rm_uu_blk(below->uu_off);
		printf("found free lower block.\n");
	}

	blkd->state = MEM_FREE;
	add_uu_blk(blkd->off, &blkd->uu_off);
}
