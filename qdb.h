# ifndef __mdl__qdb__h
# define __mdl__qdb__h
# include <mdlint.h>
# include <mdlerr.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/time.h>
# include <unistd.h>
# include <errno.h>
# include <stdio.h>
# include <pthread.h>

# define QDB_ERR_LCI 0
typedef mdl_u8_t qdb_errno_t;
struct qdb {
	int sock;
	struct sockaddr_in cli_addr, ser_addr;
	socklen_t addr_len;
	int fd;
};

typedef struct {
	mdl_u8_t *id, auth_level;
	mdl_u8_t id_bc;
	mdl_u32_t passwd;
} user_t;

enum {
	MEM_USED,
	MEM_FREE
};

struct mem_blkd {
    mdl_u16_t off, uu_off;
    mdl_u16_t above, below;
    mdl_uint_t bc;
    mdl_u8_t state;
};

struct qdb_mem {
	mdl_u16_t *uu_blks;
	mdl_uint_t uu_blk_c;
	mdl_u8_t *p;
	mdl_u16_t bc;
	mdl_uint_t off;
};

struct blkd {
	mdl_uint_t size;
	mdl_u8_t is_free;
	mdl_u32_t next;
};

struct qdb_record {
	mdl_u32_t f_off, no;
	mdl_uint_t size;
};

struct qdb_pile {
	struct qdb_record **records;
	mdl_uint_t no_records;
	mdl_u32_t id;
};

void* _qdb_mem_alloc(struct qdb*, mdl_uint_t);
void _qdb_mem_free(struct qdb*, void*);
char const* qdb_errno_str(qdb_errno_t);

mdl_err_t _qdb_creat_pile(struct qdb*, mdl_u32_t**, struct qdb_pile**);
mdl_err_t _qdb_del_pile(struct qdb*, mdl_u32_t);
mdl_err_t _qdb_add_record(struct qdb*, struct qdb_pile*, mdl_u32_t**, mdl_uint_t);
mdl_err_t _qdb_rm_record(struct qdb*, struct qdb_pile*, mdl_u32_t);
mdl_err_t _qdb_resize_record(struct qdb*, struct qdb_pile*, mdl_u32_t, mdl_uint_t);
mdl_err_t _qdb_set_record(struct qdb*, struct qdb_pile*, void*, mdl_uint_t, mdl_uint_t, mdl_u32_t);
mdl_err_t _qdb_get_record(struct qdb*, struct qdb_pile*, void*, mdl_uint_t, mdl_uint_t, mdl_u32_t);
struct qdb_pile* qdb_get_pile(mdl_u32_t);

mdl_err_t qdb_creat_pile(struct qdb*, mdl_u32_t*);
mdl_err_t qdb_del_pile(struct qdb*, mdl_u32_t);
mdl_err_t qdb_add_record(struct qdb*, mdl_u32_t*, mdl_u32_t, mdl_uint_t);
mdl_err_t qdb_rm_record(struct qdb*, mdl_u32_t, mdl_u32_t);
mdl_err_t qdb_set_record(struct qdb*, mdl_u32_t, void*, mdl_uint_t, mdl_uint_t, mdl_u32_t);
mdl_err_t qdb_get_record(struct qdb*, mdl_u32_t, void*, mdl_uint_t, mdl_uint_t, mdl_u32_t);

mdl_err_t qdb_mem_alloc(struct qdb*, void**, mdl_uint_t);
mdl_err_t qdb_mem_free(struct qdb*, void*);
mdl_err_t qdb_mem_set(struct qdb*, void*, void*, mdl_uint_t, mdl_uint_t);
mdl_err_t qdb_mem_get(struct qdb*, void*, void*, mdl_uint_t, mdl_uint_t);
mdl_err_t qdb_shutdown(struct qdb*);

user_t* _qdb_find_user(mdl_u8_t const*, mdl_u8_t const);
mdl_err_t _qdb_add_user(mdl_u8_t const*, mdl_u8_t const, mdl_u32_t);
mdl_err_t _qdb_del_user(mdl_u8_t const*, mdl_u8_t const);
mdl_err_t qdb_del_user(struct qdb*, mdl_u8_t const*, mdl_u8_t const);
mdl_err_t qdb_is_err(struct qdb*, mdl_err_t*);
mdl_err_t qdb_req_errno(struct qdb*, qdb_errno_t*);
mdl_err_t qdb_logout(struct qdb*);
mdl_err_t qdb_alive(struct qdb*);
mdl_err_t qdb_add_user(struct qdb*, mdl_u8_t const*, mdl_u8_t const, mdl_u32_t);
mdl_err_t qdb_login(struct qdb*, mdl_u8_t const*, mdl_u8_t const, mdl_u32_t);
mdl_err_t qdb_connect(struct qdb*, in_addr_t);
mdl_err_t qdb_close(struct qdb*);
mdl_err_t qdb_init(struct qdb*);
mdl_err_t qdb_begin(struct qdb*);
mdl_err_t qdb_de_init(struct qdb*);
# endif /*__mdl__qdb__h*/
