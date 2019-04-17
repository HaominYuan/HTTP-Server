#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "http_request.h"
#include "priority_queue.h"
#include "timer.h"
#include "http.h"

#define MAXEVENTS 1024
#define TIMEOUT_DEFAULT 500


int main(int argc, char *argv[]) {
		int rc, epfd, n, time, listenfd, infd, i, fd;
		struct conf_s cf;
		struct sigaction sa;
		struct epoll_event event, *events;
		struct http_request_s *request;

		/* 基本配置，这些配置可以从文件中读取或者从命令行中提取 */
		cf.root = malloc(sizeof(char) * 1024);
		cf.port = 3000;
		cf.thread_num = 4;
		
		/* 对一个已关闭的socket调用两次write，第二次将会生成SIGPIPE信号，这里将它屏蔽 */
		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = SIG_IGN;
		sa.sa_flags = 0;
		sigemptyset(&sa.sa_mask);
		if (sigaction(SIGPIPE, &sa, NULL)) {
			perror("install signal handler for SIGPIPE failed");
			return 0;
		}

		/* 被动打开端口 */
		listenfd = open_listenfd(cf.port);
		rc = make_socket_non_blocking(listenfd);
		if (rc != 0) {
			printf("make_socket_non_blocking error!\n");
			return -1;
		}
		
		/* epoll_create 这个参数已经废弃了，但是这个参数需要大于零 */
		epfd = epoll_create(1);
		events = malloc(sizeof(struct epoll_event) * MAXEVENTS);
		if (epfd < 0) {
			perror("epoll_create() error");
			return -1;
		}


		/* request 在这里设置 */
		request = malloc(sizeof(*request));
		init_request(request, listenfd, epfd, &cf);

		event.data.ptr = request;
		event.events = EPOLLIN | EPOLLET;
		epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);

		timer_init();

		printf("listenfd: %d  epfd: %d\n", listenfd, epfd);
		time = -1;
		/* 计时器在这里启动优先队列 */
		while (1) {
			// time = find_timer();
			//printf("%d\n", time);
			n = epoll_wait(epfd, events, MAXEVENTS, time);
			handle_expire_timers();
			
			for (i = 0; i < n; i++) {
				struct http_request_s *r = events[i].data.ptr;
				fd = r->fd;
				if (listenfd == fd) {
					while (1) {
						infd = accept(listenfd, NULL, NULL);
						if (infd < 0) {
							if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
								break;
							else {
								printf("accept error\n");
								break;
							}
						}
						rc = make_socket_non_blocking(infd);
						struct http_request_s *re = malloc(sizeof(*re));
						init_request(re, infd, epfd, &cf);
						event.data.ptr = re;
						event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
						epoll_ctl(epfd, EPOLL_CTL_ADD, infd, &event);
						add_timer(re, TIMEOUT_DEFAULT, http_close_conn);
					}
				} else {
					if ((events[i].events & EPOLLERR) ||
							(events[i].events & EPOLLHUP) ||
							(!(events[i].events & EPOLLIN))) {
						printf("epoll error fd: %d", r->fd);
						close(fd);
						continue;
					}
					do_request(events[i].data.ptr);
				}
			} // for
		} // while
		return 0;
}
