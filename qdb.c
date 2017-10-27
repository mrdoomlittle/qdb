# include "qdb.h"
# include <malloc.h>
# include <string.h>
# include <signal.h>
# include <stdlib.h>
# include <fcntl.h>
# include <sys/stat.h>
extern void **users;
extern struct qdb_mem mem;
extern struct qdb_pile **piles;
extern mdl_uint_t no_piles;
extern mdl_uint_t f_off;
mdl_u32_t qdb_hash(mdl_u8_t const *__key, mdl_uint_t __bc) {
	mdl_u8_t const *itr = __key;
	mdl_u32_t ret_val = 2<<(__bc>>2);
	while(itr != __key+__bc) {
		ret_val = (((ret_val>>1)+1)*(ret_val<<2))+(*itr*(((itr-__key)+1)<<1));
		itr++;
	}

	return ret_val;
}

void* qdb_mem_dupe(void *__p, mdl_uint_t __bc) {
	void *ret = malloc(__bc);
	memcpy(ret, __p, __bc);
	return ret;
}

enum {
	_msg_login,
	_msg_logout,
	_msg_alive,
	_msg_add_user,
	_msg_del_user,
	_msg_is_err,
	_msg_req_errno,
	_msg_mem_alloc,
	_msg_mem_free,
	_msg_mem_set,
	_msg_mem_get,
	_msg_shutdown,
	_msg_creat_pile,
	_msg_del_pile,
	_msg_add_record,
	_msg_rm_record,
	_msg_set_record,
	_msg_get_record
};

char const* qdb_errno_str(qdb_errno_t __errno) {
	switch(__errno) {
		case QDB_ERR_LCI: return "login credentials incorrect";
	}
	return "unknown";
}

struct qdb *_qdb = NULL;
void do_shutdown() {
    shutdown(_qdb->sock, SHUT_RDWR);
	qdb_de_init(_qdb);
	exit(0);
}

void static ctrl_c(int __sig) {
	if (!_qdb) return;
	do_shutdown();
}

mdl_err_t qdb_init(struct qdb *__qdb) {
	_qdb = __qdb;
	signal(SIGINT, &ctrl_c);

	if ((__qdb->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "qdb, failed to open socket, errno: %d\n", errno);
		return MDL_FAILURE;
	}

	__qdb->ser_addr.sin_family = AF_INET;
	__qdb->ser_addr.sin_addr.s_addr = htons(INADDR_ANY);
	__qdb->ser_addr.sin_port = htons(21299);
	__qdb->addr_len = sizeof(struct sockaddr_in);

	if (bind(__qdb->sock, (struct sockaddr*)&__qdb->ser_addr, __qdb->addr_len) < 0) {
		fprintf(stderr, "qdb, failed to bind socket, errno: %d\n", errno);
		return MDL_FAILURE;
	}

	struct timeval tv = {
		.tv_sec = 10,
		.tv_usec = 0
	};

	if (setsockopt(__qdb->sock, SOL_SOCKET, SO_RCVTIMEO, (void*)&tv, sizeof(struct timeval)) < 0) {
		fprintf(stderr, "qdb, failed to set recv time out.\n");
		return MDL_FAILURE;
	}

	users = (void**)malloc(0xFF*sizeof(void*));
	void **itr = users;
	while(itr != users+0xFF) *(itr++) = NULL;

	if ((__qdb->fd = open("out.db", O_CREAT|O_RDWR, S_IRUSR|S_IWUSR)) < 0) {
		fprintf(stderr, "failed to open db file.\n");
	}

	struct stat st;
	if (!access("out.db", F_OK)) {
		if (stat("out.db", &st) < 0) {
			fprintf(stderr, "failed to stat db file.\n");
		}

		if (st.st_size > 0) {
		mdl_u32_t off = 0;
		read(__qdb->fd, (void*)&off, sizeof(mdl_u32_t));
		lseek(__qdb->fd, off, SEEK_SET);

		mdl_uint_t no_piles = 0;
		read(__qdb->fd, (void*)&no_piles, sizeof(mdl_uint_t));
		printf("%u piles.\n", no_piles);
		for(;(no_piles--)>=1;) {
			mdl_u32_t *id;
			struct qdb_pile *pile;
			_qdb_creat_pile(__qdb, &id, &pile);
			read(__qdb->fd, (void*)&pile->no_records, sizeof(mdl_uint_t));
			printf("%u records.\n", pile->no_records);
			if (!pile->no_records) continue;
			pile->records = (struct qdb_record**)malloc(pile->no_records*sizeof(struct qdb_records*));

			struct qdb_record **itr = pile->records;
			for (;itr != pile->records+pile->no_records;itr++) {
				*itr = (struct qdb_record*)malloc(sizeof(struct qdb_record));
				read(__qdb->fd, (void*)&((*itr)->f_off), sizeof(mdl_u32_t));
				struct blkd blk;
				pread(__qdb->fd, (void*)&blk, sizeof(struct blkd), (*itr)->f_off-sizeof(struct blkd));
				(*itr)->size = blk.size;
			}
		}
		f_off = off;
		}
	} else f_off += sizeof(mdl_u32_t);

	mem.p = (mdl_u8_t*)malloc((mem.bc = 200));
	mem.off = 0;
	mem.uu_blks = NULL;
	mem.uu_blk_c = 0;
	return MDL_SUCCESS;
}

typedef struct {
	void *p1, *p2;
} pair_t;

struct qdb_msg {
	mdl_u8_t kind;
	mdl_uint_t size;
} __attribute__((packed));

mdl_err_t qdb_snd_msg(int, struct qdb_msg*);
mdl_err_t qdb_rcv_msg(int, struct qdb_msg*);
mdl_err_t qdb_login(struct qdb *__qdb, mdl_u8_t const *__id, mdl_u8_t const __id_bc, mdl_u32_t __passwd) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_login, .size=0});
	send(__qdb->sock, (void*)&__id_bc, 1, 0);
	send(__qdb->sock, (void*)__id, __id_bc, 0);
	send(__qdb->sock, (void*)&__passwd, sizeof(mdl_u32_t), 0);
}

mdl_err_t qdb_logout(struct qdb *__qdb) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_logout, .size=0});
}

mdl_err_t qdb_alive(struct qdb *__qdb) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_alive, .size=0});
}

mdl_err_t qdb_add_user(struct qdb *__qdb, mdl_u8_t const *__id, mdl_u8_t const __id_bc, mdl_u32_t __passwd) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_add_user, .size=0});

	send(__qdb->sock, (void*)&__id_bc, 1, 0);
	send(__qdb->sock, (void*)__id, __id_bc, 0);
	send(__qdb->sock, (void*)&__passwd, sizeof(mdl_u32_t), 0);
}

mdl_err_t qdb_del_user(struct qdb *__qdb, mdl_u8_t const *__id, mdl_u8_t const __id_bc) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_del_user, .size=0});

	send(__qdb->sock, (void*)&__id_bc, 1, 0);
	send(__qdb->sock, (void*)__id, __id_bc, 0);
}

mdl_err_t qdb_is_err(struct qdb *__qdb, mdl_err_t *__err) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_is_err, .size=0});
	recv(__qdb->sock, (void*)__err, sizeof(mdl_err_t), 0);
}

mdl_err_t qdb_req_errno(struct qdb *__qdb, qdb_errno_t *__errno) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_req_errno, .size=0});
	recv(__qdb->sock, (void*)__errno, sizeof(qdb_errno_t), 0);
}

mdl_err_t qdb_snd_msg(int __sock, struct qdb_msg *__msg) {
	if (send(__sock, (void*)__msg, sizeof(struct qdb_msg), 0) <= 0)
		return MDL_FAILURE;
	return MDL_SUCCESS;
}

mdl_err_t qdb_rcv_msg(int __sock, struct qdb_msg *__msg) {
	if (recv(__sock, (void*)__msg, sizeof(struct qdb_msg), 0) <= 0)
		return MDL_FAILURE;
	return MDL_SUCCESS;
}

mdl_err_t qdb_mem_alloc(struct qdb *__qdb, void **__p, mdl_uint_t __bc) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_mem_alloc, .size=0});
	send(__qdb->sock, (void*)&__bc, sizeof(mdl_uint_t), 0);
	recv(__qdb->sock, (void*)__p, sizeof(void*), 0);
}

mdl_err_t qdb_mem_free(struct qdb *__qdb, void *__p) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_mem_free, .size=0});
	send(__qdb->sock, (void*)&__p, sizeof(void*), 0);
}

mdl_err_t qdb_mem_set(struct qdb *__qdb, void *__p, void *__buff, mdl_uint_t __off, mdl_uint_t __bc) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_mem_set, .size=0});
	send(__qdb->sock, (void*)&__p, sizeof(void*), 0);
	send(__qdb->sock, (void*)&__off, sizeof(mdl_uint_t), 0);
	send(__qdb->sock, (void*)&__bc, sizeof(mdl_uint_t), 0);
	send(__qdb->sock, (void*)__buff, __bc, 0);
}

mdl_err_t qdb_mem_get(struct qdb *__qdb, void *__p, void *__buff, mdl_uint_t __off, mdl_uint_t __bc) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_mem_get, .size=0});
	send(__qdb->sock, (void*)&__p, sizeof(void*), 0);
	send(__qdb->sock, (void*)&__off, sizeof(mdl_uint_t), 0);
	send(__qdb->sock, (void*)&__bc, sizeof(mdl_uint_t), 0);
	recv(__qdb->sock, (void*)__buff, __bc, 0);
}

mdl_err_t qdb_shutdown(struct qdb *__qdb) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_shutdown, .size=0});
}

mdl_err_t qdb_creat_pile(struct qdb *__qdb, mdl_u32_t *__id) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_creat_pile, .size=0});
	recv(__qdb->sock, (void*)__id, sizeof(mdl_u32_t), 0);
}

mdl_err_t qdb_del_pile(struct qdb *__qdb, mdl_u32_t __id) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_del_pile, .size=0});
	send(__qdb->sock, (void*)&__id, sizeof(mdl_u32_t), 0);
}

mdl_err_t qdb_add_record(struct qdb *__qdb, mdl_u32_t *__no, mdl_u32_t __pile_id, mdl_uint_t __size) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_add_record, .size=0});
	send(__qdb->sock, (void*)&__pile_id, sizeof(mdl_u32_t), 0);
	send(__qdb->sock, (void*)&__size, sizeof(mdl_uint_t), 0);
	recv(__qdb->sock, (void*)__no, sizeof(mdl_u32_t), 0);
}

mdl_err_t qdb_rm_record(struct qdb *__qdb, mdl_u32_t __no, mdl_u32_t __pile_id) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_rm_record, .size=0});
	send(__qdb->sock, (void*)&__no, sizeof(mdl_u32_t), 0);
	send(__qdb->sock, (void*)&__pile_id, sizeof(mdl_u32_t), 0);
}

mdl_err_t qdb_set_record(struct qdb *__qdb, mdl_u32_t __pile_id, void *__buff, mdl_uint_t __off, mdl_uint_t __bc, mdl_u32_t __no) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_set_record, .size=0});
	send(__qdb->sock, (void*)&__pile_id, sizeof(mdl_u32_t), 0);
	send(__qdb->sock, (void*)&__off, sizeof(mdl_uint_t), 0);
	send(__qdb->sock, (void*)&__off, sizeof(mdl_uint_t), 0);
	send(__qdb->sock, (void*)&__no, sizeof(mdl_u32_t), 0);
	send(__qdb->sock, __buff, __bc, 0);
}

mdl_err_t qdb_get_record(struct qdb *__qdb, mdl_u32_t __pile_id, void *__buff, mdl_uint_t __off, mdl_uint_t __bc, mdl_u32_t __no) {
	qdb_snd_msg(__qdb->sock, &(struct qdb_msg){.kind=_msg_get_record, .size=0});
	send(__qdb->sock, (void*)&__pile_id, sizeof(mdl_u32_t), 0);
	send(__qdb->sock, (void*)&__off, sizeof(mdl_uint_t), 0);
	send(__qdb->sock, (void*)&__off, sizeof(mdl_uint_t), 0);
	send(__qdb->sock, (void*)&__no, sizeof(mdl_u32_t), 0);
	recv(__qdb->sock, __buff, __bc, 0);
}

mdl_u8_t qdb_is_msg(struct qdb_msg *__msg, mdl_u8_t __kind) {
	return __msg->kind == __kind;
}

mdl_err_t _qdb_login(mdl_u8_t *__id, mdl_u8_t __id_bc, mdl_u32_t __passwd) {
	user_t *user = _qdb_find_user(__id, __id_bc);
	if (!user) return MDL_FAILURE;
	if (user->passwd != __passwd) return MDL_FAILURE;
	return MDL_SUCCESS;
}

void qdb_handle(struct qdb *__qdb, int __sock) {
	printf("client connected.\n");

	mdl_u8_t has_auth = 0;
	struct qdb_msg msg;
	mdl_err_t any_err = MDL_SUCCESS;
	qdb_errno_t _errno;

	_again:
	printf("user %s\n", has_auth? "is logged in":"is not logged in");
	if (qdb_rcv_msg(__sock, &msg) != MDL_SUCCESS) goto _end;

	if (qdb_is_msg(&msg, _msg_login)) {
		mdl_uint_t id_bc;
		recv(__sock, (void*)&id_bc, 1, 0);

		mdl_u8_t *id = (mdl_u8_t*)malloc(id_bc);
		recv(__sock, (void*)id, id_bc, 0);

		mdl_u32_t passwd;
		recv(__sock, (void*)&passwd, sizeof(mdl_u32_t), 0);

		any_err = _qdb_login(id, id_bc, passwd);
		if (any_err == MDL_SUCCESS) {
			has_auth = 1;
			printf("user logged in with id: %.*s.\n", id_bc, id);
		}
		free(id);
	} else if (qdb_is_msg(&msg, _msg_alive)) {
		printf("alive sent.\n");
	} else if (qdb_is_msg(&msg, _msg_is_err))
		send(__sock, (void*)&any_err, sizeof(mdl_err_t), 0);
	else if (qdb_is_msg(&msg, _msg_req_errno))
		send(__sock, (void*)&_errno, sizeof(qdb_errno_t), 0);

	if (!has_auth) goto _again;

	if (qdb_is_msg(&msg, _msg_add_user)) {
		mdl_uint_t id_bc;
		recv(__sock, (void*)&id_bc, 1, 0);

		mdl_u8_t *id = (mdl_u8_t*)malloc(id_bc);
		recv(__sock, (void*)id, id_bc, 0);

		mdl_u32_t passwd;
		recv(__sock, (void*)&passwd, sizeof(mdl_u32_t), 0);
		_qdb_add_user((mdl_u8_t*)id, id_bc, passwd);
		printf("adding user with id: %.*s.\n", id_bc, id);
		free(id);
	} else if (qdb_is_msg(&msg, _msg_del_user)) {
		mdl_uint_t id_bc;
        recv(__sock, (void*)&id_bc, 1, 0);

        mdl_u8_t *id = (mdl_u8_t*)malloc(id_bc);
        recv(__sock, (void*)id, id_bc, 0);

		_qdb_del_user((mdl_u8_t*)id, id_bc);
		printf("deleting user with id: %.*s.\n", id_bc, id);
		free(id);
	} else if (qdb_is_msg(&msg, _msg_logout)) {
		has_auth = 0;
		printf("user logged out.\n");
	} else if (qdb_is_msg(&msg, _msg_mem_alloc)) {
		mdl_uint_t bc;
		recv(__sock, (void*)&bc, sizeof(mdl_uint_t), 0);
		void *p = _qdb_mem_alloc(__qdb, bc);
		send(__sock, (void*)&p, sizeof(void*), 0);
	} else if (qdb_is_msg(&msg, _msg_mem_free)) {
		void *p;
		recv(__sock, (void*)&p, sizeof(void*), 0);
		_qdb_mem_free(__qdb, p);
	} else if (qdb_is_msg(&msg, _msg_mem_set)) {
		void *p;
		mdl_uint_t off, bc;
		recv(__sock, (void*)&p, sizeof(void*), 0);
		recv(__sock, (void*)&off, sizeof(mdl_uint_t), 0);
		recv(__sock, (void*)&bc, sizeof(mdl_uint_t), 0);
		recv(__sock, (void*)((mdl_u8_t*)p+off), bc, 0);
	} else if (qdb_is_msg(&msg, _msg_mem_get)) {
		void *p;
		mdl_uint_t off, bc;
		recv(__sock, (void*)&p, sizeof(void*), 0);
		recv(__sock, (void*)&off, sizeof(mdl_uint_t), 0);
		recv(__sock, (void*)&bc, sizeof(mdl_uint_t), 0);
		send(__sock, (void*)((mdl_u8_t*)p+off), bc, 0);
	} else if (qdb_is_msg(&msg, _msg_shutdown)) do_shutdown();
	else if (qdb_is_msg(&msg, _msg_creat_pile)) {
		mdl_u32_t *id;
		_qdb_creat_pile(__qdb, &id, NULL);
		send(__sock, (void*)id, sizeof(mdl_u32_t), 0);
	} else if (qdb_is_msg(&msg, _msg_del_pile)) {
		mdl_u32_t id;
		recv(__sock, (void*)&id, sizeof(mdl_u32_t), 0);
		_qdb_del_pile(__qdb, id);
	} else if (qdb_is_msg(&msg, _msg_add_record)) {
		mdl_u32_t pile_id, *no;
		mdl_uint_t size;
		recv(__sock, (void*)&pile_id, sizeof(mdl_u32_t), 0);
		recv(__sock, (void*)&size, sizeof(mdl_uint_t), 0);

		_qdb_add_record(__qdb, qdb_get_pile(pile_id), &no, size);
		send(__sock, (void*)no, sizeof(mdl_u32_t), 0);
	} else if (qdb_is_msg(&msg, _msg_rm_record)) {
		mdl_u32_t pile_id, no;
		recv(__sock, (void*)&no, sizeof(mdl_u32_t), 0);
		recv(__sock, (void*)&pile_id, sizeof(mdl_u32_t), 0);
		_qdb_rm_record(__qdb, qdb_get_pile(pile_id), no);
	} else if (qdb_is_msg(&msg, _msg_set_record)) {
		mdl_u32_t pile_id, no;
		mdl_uint_t off, bc;
		recv(__sock, (void*)&pile_id, sizeof(mdl_u32_t), 0);
		recv(__sock, (void*)&off, sizeof(mdl_uint_t), 0);
		recv(__sock, (void*)&bc, sizeof(mdl_uint_t), 0);
		recv(__sock, (void*)&no, sizeof(mdl_u32_t), 0);
		void *buff = malloc(bc);
		recv(__sock, buff, bc, 0);
		_qdb_set_record(__qdb, qdb_get_pile(pile_id), buff, off, bc, no);
		free(buff);
	} else if (qdb_is_msg(&msg, _msg_get_record)) {
		mdl_u32_t pile_id, no;
		mdl_uint_t off, bc;
		recv(__sock, (void*)&pile_id, sizeof(mdl_u32_t), 0);
		recv(__sock, (void*)&off, sizeof(mdl_uint_t), 0);
		recv(__sock, (void*)&bc, sizeof(mdl_uint_t), 0);
		recv(__sock, (void*)&no, sizeof(mdl_u32_t), 0);
		void *buff = malloc(bc);
		_qdb_get_record(__qdb, qdb_get_pile(pile_id), buff, off, bc, no);
		send(__sock, buff, bc, 0);
		free(buff);
	}
	goto _again;
	_end:

	close(__sock);
	printf("client disconnected.\n");
}

mdl_err_t static qdb_listen(struct qdb *__qdb) {
	if (listen(__qdb->sock, 24) < 0) return MDL_FAILURE;
	return MDL_SUCCESS;
}

mdl_err_t static qdb_accept(struct qdb *__qdb, int *__sock) {
	if ((*__sock = accept(__qdb->sock, (struct sockaddr*)&__qdb->cli_addr, &__qdb->addr_len)) < 0)
		return MDL_FAILURE;
	return MDL_SUCCESS;
}

mdl_err_t qdb_connect(struct qdb *__qdb, in_addr_t __addr) {
	if ((__qdb->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "qdb, failed to open sock, errno: %d\n", errno);
		return MDL_FAILURE;
	}

	__qdb->ser_addr.sin_family = AF_INET;
	__qdb->ser_addr.sin_addr.s_addr = __addr;
	__qdb->ser_addr.sin_port = htons(21299);
	__qdb->addr_len = sizeof(struct sockaddr_in);

	if (connect(__qdb->sock, (struct sockaddr*)&__qdb->ser_addr, __qdb->addr_len) < 0) {
		fprintf(stderr, "qdb, failed to connect, errno: %d\n", errno);
		return MDL_FAILURE;
	}
	return MDL_SUCCESS;
}

mdl_err_t qdb_close(struct qdb *__qdb) {
	close(__qdb->sock);
}

mdl_u8_t static h_prox_inited = 0;
void* qdb_handle_proxy(void *__arg) {
	struct qdb *_qdb = (struct qdb*)((pair_t*)__arg)->p1;
	int sock = *(int*)((pair_t*)__arg)->p2;
	h_prox_inited = 1;

	qdb_handle(_qdb, sock);
}

mdl_err_t qdb_begin(struct qdb *__qdb) {
	do {
		if (qdb_listen(__qdb) != MDL_SUCCESS) continue;

		int sock;
		if (qdb_accept(__qdb, &sock) != MDL_SUCCESS) continue;

		pair_t param = {.p1=(void*)__qdb, .p2=&sock};
		pthread_t t;
		pthread_create(&t, NULL, qdb_handle_proxy, &param);
		while(!h_prox_inited);
		h_prox_inited = 0;
	} while(1);
}

mdl_err_t qdb_de_init(struct qdb *__qdb) {
	printf("%u\n", f_off);
	lseek(__qdb->fd, 0, SEEK_SET);
	write(__qdb->fd, (void*)&f_off, sizeof(mdl_u32_t));

	lseek(__qdb->fd, f_off, SEEK_SET);
	write(__qdb->fd, (void*)&no_piles, sizeof(mdl_uint_t));
	struct qdb_pile **itr = piles;
	while(itr != piles+no_piles) {
		struct qdb_pile *pile = *itr;
		write(__qdb->fd, (void*)&pile->no_records, sizeof(mdl_uint_t));
		struct qdb_record **_itr = pile->records;
		while(_itr != pile->records+pile->no_records) {
			write(__qdb->fd, (void*)&(*_itr)->f_off, sizeof(mdl_u32_t));
			_itr++;
		}
		itr++;
	}

	ftruncate(__qdb->fd, lseek(__qdb->fd, 0, SEEK_CUR));

	close(__qdb->sock);
	close(__qdb->fd);
}
