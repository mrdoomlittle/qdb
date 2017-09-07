# ifndef __mdl__qdb__h
# define __mdl__qdb__h
# include <mdlint.h>
# include <mdlerr.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <errno.h>
# include <stdio.h>
# include <pthread.h>
struct qdb {
	int sock;
	struct sockaddr_in cli_addr, ser_addr;
	socklen_t addr_len;
};

mdl_err_t qdb_connect(struct qdb*, in_addr_t);
mdl_err_t qdb_init(struct qdb*);
mdl_err_t qdb_begin(struct qdb*);
mdl_err_t qdb_de_init(struct qdb*);
# endif /*__mdl__qdb__h*/
