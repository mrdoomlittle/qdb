# include "qdb.h"
enum {
	_msg_login
};

mdl_err_t qdb_init(struct qdb *__qdb) {
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
	return MDL_SUCCESS;
}

mdl_err_t qdb_login(struct qdb *__qdb) {


}

typedef struct {
	void *p1, *p2;
} pair_t;

struct qdb_msg {
	mdl_u8_t kind;
	mdl_uint_t size;
} __attribute__((packed));

mdl_err_t qdb_snd_msg(int __sock, struct qdb_msg *__msg) {
	if (send(__sock, (void*)__msg, sizeof(struct qdb_msg), 0) < 0)
		return MDL_FAILURE;
	return MDL_SUCCESS;
}

mdl_err_t qdb_rcv_msg(int __sock, struct qdb_msg *__msg) {
	if (recv(__sock, (void*)__msg, sizeof(struct qdb_msg), 0) < 0)
		return MDL_FAILURE;
	return MDL_SUCCESS;
}

void* qdb_handle(void *__arg) {
	struct qdb *__qdb = (struct qdb*)((pair_t*)__arg)->p1;
	int __sock = *(int*)((pair_t*)__arg)->p2;
	printf("client connected.\n");

	while(1) {

		
	}

	close(__sock);
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

mdl_err_t qdb_begin(struct qdb *__qdb) {
	do {
		if (qdb_listen(__qdb) != MDL_SUCCESS) continue;

		int sock;
		if (qdb_accept(__qdb, &sock) != MDL_SUCCESS) continue;

		pair_t param = {.p1 = (void*)__qdb, .p2 = &sock};
		pthread_t t;
		pthread_create(&t, NULL, qdb_handle, &param);
	} while(1);
}

mdl_err_t qdb_de_init(struct qdb *__qdb) {

}
