# include "qdb.h"
# include <string.h>
# include <stdio.h>
# include <time.h>
# include <unistd.h>
extern void* qdb_mem_dupe(void*, mdl_uint_t);
extern mdl_u32_t qdb_hash(mdl_u8_t const*, mdl_uint_t);
char* read_part(char **__itr, char *__buff) {
	char *buff_itr = __buff;
	while(**__itr != ' ' && **__itr != '\n') *(buff_itr++) = *((*__itr)++);
	*buff_itr = '\0';
	(*__itr)++;
	return __buff;
}

char* read_part_and_dupe(char **__itr, char *__buff) {
	char *part = read_part(__itr, __buff);
	return qdb_mem_dupe(part, strlen(part));
}

int main(void) {
	struct qdb _qdb;
	in_addr_t addr;
	addr = inet_addr("127.0.0.1");
	qdb_connect(&_qdb, addr);

	char buff[200];
	char *line = NULL;
	struct timespec begin, end;
	clock_gettime(CLOCK_MONOTONIC, &begin);

	struct timeval tv = {
		.tv_sec = 2,
		.tv_usec = 0
	};

	while(1) {
		ssize_t nread;

		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		int ret = select(1, &fds, NULL, NULL, &tv);
		if (ret == 0) goto _r;
		if (getline(&line, &nread, stdin) <= 0) goto _r;

		char *itr = line;

		char *part = read_part(&itr, buff);
		if (!strcmp(part, "login")) {
			char *id = read_part_and_dupe(&itr, buff);
			char *passwd = read_part_and_dupe(&itr, buff);
			printf("id: %s, passwd: %s\n", id, passwd);
			if (qdb_login(&_qdb, (mdl_u8_t const*)id, strlen(id), qdb_hash(passwd, strlen(passwd))) != MDL_SUCCESS) {
				printf("failed to login wong password or user does not exist.\n");
			}
		} else if (!strcmp(part, "logout")) qdb_logout(&_qdb);
		else if (!strcmp(part, "quit")) return 0;
		else if (!strcmp(part, "add_user")) {
			char *id = read_part_and_dupe(&itr, buff);
			char *passwd = read_part_and_dupe(&itr, buff);
			qdb_add_user(&_qdb, (mdl_u8_t const*)id, strlen(id), qdb_hash(passwd, strlen(passwd)));
		} else if (!strcmp(part, "del_user")) {
			char *id = read_part_and_dupe(&itr, buff);
			qdb_del_user(&_qdb, (mdl_u8_t const*)id, strlen(id));
		} else {
			_r:
			clock_gettime(CLOCK_MONOTONIC, &end);
			if (end.tv_sec-begin.tv_sec > 2) {
				qdb_alive(&_qdb);
			} else
				continue;
		}

		clock_gettime(CLOCK_MONOTONIC, &begin);
	}
	return 0;
}
