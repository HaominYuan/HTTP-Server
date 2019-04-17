#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "http.h"
#include "http_request.h"
#include "timer.h"
#include "http_parse.h"

#define SHORTLINE 512
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static char *ROOT = NULL;

mime_type_t mine[] = {
	{".html", "text/html"},
	{".xml", "text/xml"},
	{".xhtml", "application/xhtml+xml"},
	{".txt", "text/plain"},
	{".rtf", "application/rtf"},
	{".pdf", "application/pdf"},
	{".word", "application/msword"},
	{".png", "image/png"},
	{".gif", "image/gif"},
	{".jpg", "image/jpeg"},
	{",jpeg", "image/jpeg"},
	{".au", "audio/basic"},
	{".mpeg", "video/mpeg"},
	{".mpg", "video/mpeg"},
	{".avi", "video/x-msvideo"},
	{".gz", "application/x-gzip"},
	{".tar", "application/x-tar"},
	{".css", "text/css"},
	{NULL, "text/plain"}
};

void do_request(void *ptr) {
	printf("do request\n");
	struct http_request_s *r = ptr;
	int fd = r->fd;
	int rc, n;
	char filename[SHORTLINE];
	struct stat sbuf;
	ROOT = r->root;
	char *plast = NULL;
	size_t remain_size;

	/* 将定时器删除，否则资源容易被释放 */
	del_timer(r);
	for (;;) {
		plast = &r->buf[r->last % MAX_BUF];
		remain_size = MIN(MAX_BUF - (r->last - r->pos) - 1, MAX_BUF - r->last % MAX_BUF);
		n = read(fd, plast, remain_size);

		/* 表示已经读取完了 */
		if (n == 0) {
			printf("read return 0, ready to close fd %d, remain_size = %zu", fd, remain_size);
			goto err;
		}

		/* 出现错误，如果错误是 EAGAIN 表示缓冲区中的数据已经读取完了，此时必须退出循环，否则容易卡死 */
		if (n < 0) {
			if (errno != EAGAIN) {
				printf("read err, and errno = %d\n", errno);
				goto err;
			}
			break;
		}

		r->last += n;

		rc = http_parse_request_line(r);
		if (rc == EAGAIN) {
			continue;
		} else if (rc != 0) {
			printf("rc != ok\n");
			//goto err;
		}

		printf("Method: %d\n", r->method);
		char *begin = r->uri_start;
		while (begin != r->uri_end) {
			printf("%c", *begin);
			begin = begin + 1;
		}
		printf("\n");
	}


err:
close:
	rc = http_close_conn(r);
}
