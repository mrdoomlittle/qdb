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

struct qdb {
	int sock;
	struct sockaddr_in cli_addr, ser_addr;
	socklen_t addr_len;
	int fd;
};

typedef struct {
	mdl_u8_t *id;
	mdl_u8_t id_bc;

	mdl_u32_t passwd;
} user_t;

void* qdb_mem_alloc(struct qdb*, mdl_uint_t);
void qdb_mem_free(struct qdb*, void*);

user_t* _qdb_find_user(mdl_u8_t const*, mdl_u8_t const);
mdl_err_t _qdb_add_user(mdl_u8_t const*, mdl_u8_t const, mdl_u32_t);
mdl_err_t _qdb_del_user(mdl_u8_t const*, mdl_u8_t const);
mdl_err_t qdb_del_user(struct qdb*, mdl_u8_t const*, mdl_u8_t const);
mdl_err_t qdb_logout(struct qdb*);
mdl_err_t qdb_alive(struct qdb*);
mdl_err_t qdb_add_user(struct qdb*, mdl_u8_t const*, mdl_u8_t const, mdl_u32_t);
mdl_err_t qdb_login(struct qdb*, mdl_u8_t const*, mdl_u8_t const, mdl_u32_t);
mdl_err_t qdb_connect(struct qdb*, in_addr_t);
mdl_err_t qdb_init(struct qdb*);
mdl_err_t qdb_begin(struct qdb*);
mdl_err_t qdb_de_init(struct qdb*);
# endif /*__mdl__qdb__h*/
