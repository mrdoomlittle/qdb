# include "qdb.h"
# include <malloc.h>
# include <string.h>
# include <signal.h>
# include <stdlib.h>
# include <fcntl.h>
extern void **users;
extern mdl_u64_t f_off;
extern mdl_u64_t last_blk;
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
	_msg_del_user
};

struct qdb *_qdb = NULL;
void static ctrl_c(int __sig) {
	if (!_qdb) return;

	close(_qdb->sock);
	exit(0);
}

mdl_err_t qdb_init(struct qdb *__qdb) {
	signal(SIGINT, &ctrl_c);
	_qdb = __qdb;

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

	mdl_u8_t existing_data = 0;

	if (!access("out.raw", F_OK)) existing_data = 1;
	if ((__qdb->fd = open("out.raw", O_RDWR, O_CREAT)) < 0) {
		fprintf(stderr, "qdb, failed to open db file.\n");
		return MDL_FAILURE;
	}

	if (existing_data) {
		read(__qdb->fd, (void*)&f_off, sizeof(mdl_u64_t));
		read(__qdb->fd, (void*)&last_blk, sizeof(mdl_u64_t));
	}

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

	mdl_err_t res;
	recv(__qdb->sock, (void*)&res, sizeof(mdl_err_t), 0);
	return res;
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

		mdl_err_t res = _qdb_login(id, id_bc, passwd);
		send(__sock, (void*)&res, sizeof(mdl_err_t), 0);
		if (res == MDL_SUCCESS) {
			has_auth = 1;
			printf("user logged in with id: %.*s.\n", id_bc, id);
		}
		free(id);
	} else if (qdb_is_msg(&msg, _msg_alive)) {
		printf("alive sent.\n");
	}

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

}
