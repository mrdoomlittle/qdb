# include "qdb.h"
# include <string.h>
# include <stdio.h>
# include <time.h>
# include <unistd.h>
extern void* qdb_mem_dupe(void*, mdl_uint_t);
extern mdl_u32_t qdb_hash(mdl_u8_t const*, mdl_uint_t);
char* read_part(char **__itr, char *__buff) {
	char *buff_itr = __buff;
	while(**__itr != ' ' && **__itr != '\n' && **__itr != '\0') *(buff_itr++) = *((*__itr)++);
	*buff_itr = '\0';
	(*__itr)++;
	return __buff;
}

char* read_part_and_dupe(char **__itr, char *__buff) {
	char *part = read_part(__itr, __buff);
	return qdb_mem_dupe(part, strlen(part));
}

# include <stdlib.h>
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
		if (!strcmp(part, "help")) {
			printf("login[id, password]\nlogout\nquit\nadd_user[id, password]\ndel_user[id]\n");
		} else if (!strcmp(part, "login")) {
			char *id = read_part_and_dupe(&itr, buff);
			char *passwd = read_part_and_dupe(&itr, buff);
			printf("id: %s, passwd: %s\n", id, passwd);
			qdb_login(&_qdb, (mdl_u8_t const*)id, strlen(id), qdb_hash(passwd, strlen(passwd)));
			mdl_err_t err;
			qdb_is_err(&_qdb, &err);
			if (err != MDL_SUCCESS) {
				qdb_errno_t _errno;
				qdb_req_errno(&_qdb, &_errno);
				printf("failed to login, err: %s\n", qdb_errno_str(_errno));
			} else {
				printf("success your now loged in as %s.\n", id);
			}
		} else if (!strcmp(part, "logout")) qdb_logout(&_qdb);
		else if (!strcmp(part, "quit")) {
			qdb_close(&_qdb);
			return 0;
		} else if (!strcmp(part, "add_user")) {
			char *id = read_part_and_dupe(&itr, buff);
			char *passwd = read_part_and_dupe(&itr, buff);
			qdb_add_user(&_qdb, (mdl_u8_t const*)id, strlen(id), qdb_hash(passwd, strlen(passwd)));

		} else if (!strcmp(part, "del_user")) {
			char *id = read_part_and_dupe(&itr, buff);
			qdb_del_user(&_qdb, (mdl_u8_t const*)id, strlen(id));
		} else if (!strcmp(part, "mem_alloc")) {
			char *bc = read_part(&itr, buff);
			void *p;
			qdb_mem_alloc(&_qdb, &p, atoi(bc));
			printf("%p\n", p);
		} else if (!strcmp(part, "mem_free")) {
			char *p = read_part(&itr, buff);
			qdb_mem_free(&_qdb, (void*)strtol(p, NULL, 16));
		} else if (!strcmp(part, "mem_set")) {
			void *p = (void*)strtol(read_part(&itr, buff), NULL, 16);
			mdl_uint_t off = atoi(read_part(&itr, buff));
			mdl_uint_t bc = atoi(read_part(&itr, buff));

			mdl_u8_t *buff = (mdl_u8_t*)malloc(bc);
			mdl_u8_t *buf_itr = buff;
			char val[5] = {'\0', '\0', '\0', '\0', '\0'};
			while(buf_itr != buff+bc) {
				*(mdl_u32_t*)val = *(mdl_u32_t*)itr;
				itr+=sizeof(mdl_u32_t);
				*(buf_itr++) = strtol(val, NULL, 16);
				if (*(itr++) != ',') {
					printf("error, got: %c\n", *itr);
					break;
				}
			}

			qdb_mem_set(&_qdb, p, buff, off, bc);
			free(buff);
		} else if (!strcmp(part, "mem_get")) {
			void *p = (void*)strtol(read_part(&itr, buff), NULL, 16);
			mdl_uint_t off = atoi(read_part(&itr, buff));
			mdl_uint_t bc = atoi(read_part(&itr, buff));
			mdl_u8_t *buff = (mdl_u8_t*)malloc(bc);
			qdb_mem_get(&_qdb, p, buff, off, bc);
			mdl_u8_t *buf_itr = buff;
			while(buf_itr != buff+bc) {
				mdl_uint_t _off = buf_itr-buff;
				if ((((_off+1)>>4)-(_off>>4)) || (buf_itr+1 == buff+bc))
					printf("%x\n", *buf_itr);
				else
					printf("%x, ", *buf_itr);
				buf_itr++;
			}

			free(buff);
		} else if (!strcmp(part, "shutdown")) {
			qdb_shutdown(&_qdb);
			qdb_close(&_qdb);
			return 0;
		} else if (!strcmp(part, "creat_pile")) {
			mdl_u32_t id;
			qdb_creat_pile(&_qdb, &id);
			printf("id: %u\n", id);
		} else if (!strcmp(part, "del_pile")) {
			mdl_u32_t id = atoi(read_part(&itr, buff));
			qdb_del_pile(&_qdb, id);
		} else if (!strcmp(part, "add_record")) {
			mdl_u32_t pile_id = atoi(read_part(&itr, buff));
			mdl_uint_t size = atoi(read_part(&itr, buff));
			mdl_u32_t no;
			qdb_add_record(&_qdb, &no, pile_id, size);
			printf("no: %u\n", no);
		} else if (!strcmp(part, "rm_record")) {
			mdl_u32_t no = atoi(read_part(&itr, buff));
			mdl_u32_t pile_id = atoi(read_part(&itr, buff));
			qdb_rm_record(&_qdb, no, pile_id);
		} else if (!strcmp(part, "set_record")) {

		} else if (!strcmp(part, "get_record")) {

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
