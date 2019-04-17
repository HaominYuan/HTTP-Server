#include "http_parse.h"
#include "http.h"
#include <sys/types.h>
#include <errno.h>

#define str_cmp(m, c0, c1, c2, c3) \
	*(int32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

int http_parse_request_line(struct http_request_s *r) {
	u_char ch, *p, *m;
	size_t pi;

	enum {
		sw_start = 0,
		sw_method,
		sw_spaces_before_uri,
		sw_after_slash_in_uri,
		sw_http,
		sw_http_H,
		sw_http_HT,
		sw_http_HTT,
		sw_http_HTTP,
		sw_first_major_digit,
		sw_major_digit,
		sw_first_minor_digit,
		sw_minor_digit,
		sw_spaces_after_digit,
		sw_almost_done
	} state;

	state = r->state;

	for (pi = r->pos; pi < r->last; pi++) {
		p = (u_char *)&r->buf[pi % MAX_BUF];
		ch = *p;

		switch (state) {
			case sw_start:
				r->request_start = p;
				if (ch == CR || ch == LF) {
					break;
				}

				if ((ch < 'A' || ch > 'Z') && ch != ' ') {
					return HTTP_PARSE_INVALID_METHOD;
				}

				state = sw_method;
				break;

			case sw_method:
				if (ch == ' ') {
					r->method_end = p;
					m = r->request_start;

					switch (p - m) {
						case 3:
							if (str_cmp(m, 'G', 'E', 'T', ' ')) {
								r->method = HTTP_GET;
								break;
							}
							break;
						case 4:
							if (str_cmp(m, 'P', 'O', 'S', 'T')) {
								printf("here\n");
								r->method = HTTP_POST;
								break;
							}

							if (str_cmp(m, 'H', 'E', 'A', 'D')) {
								r->method = HTTP_HEAD;
								break;
							}
							break;
						default:
							r->method = HTTP_UNKNOWN;
							break;
					}
					state = sw_spaces_before_uri;
					break;
				}


				if ((ch < 'A' || ch > 'Z') && ch != ' ') {
					return HTTP_PARSE_INVALID_METHOD;
				}
				break;

			case sw_spaces_before_uri:
				if (ch == '/') {
					r->uri_start = p;
					state = sw_after_slash_in_uri;
					break;
				}

				switch (ch) {
					case ' ':
						break;
					default:
						return HTTP_PARSE_INVALID_REQUEST;
				}
				break;

			case sw_after_slash_in_uri:
				switch (ch) {
					case ' ':
						r->uri_end = p;
						state = sw_http;
						break;
					default:
						break;
				}
				break;

			case sw_http:
				switch (ch) {
					case ' ':
						break;
					case 'H':
						state = sw_http_H;
						break;
					default:
						return HTTP_PARSE_INVALID_REQUEST;
				}
				break;

			case sw_http_H:
				if (ch == 'T') {
					state = sw_http_HT;
				} else {
					return HTTP_PARSE_INVALID_REQUEST;
				}
				break;

			case sw_http_HT:
				if (ch == 'T') {
					state = sw_http_HTT;
				} else {
					return HTTP_PARSE_INVALID_REQUEST;
				}
				break;

			case sw_http_HTT:
				if (ch == 'P') {
					state = sw_http_HTTP;
				} else {
					return HTTP_PARSE_INVALID_REQUEST;
				}
				break;

			case sw_http_HTTP:
				if (ch == '/') {
					state = sw_first_major_digit;
					break;
				} else {
					return HTTP_PARSE_INVALID_REQUEST;
				}
				break;

			case sw_first_major_digit:
				if (ch < '1' || ch > '9') {
					return HTTP_PARSE_INVALID_REQUEST;
				}
				r->http_major = ch - '0';
				state = sw_major_digit;
				break;

			case sw_major_digit:
				if (ch == '.') {
					state = sw_first_minor_digit;
					break;
				}

				if (ch < '0' || ch > '9') {
					return HTTP_PARSE_INVALID_REQUEST;
				}

				r->http_major = r->http_major * 10 + ch - '0';
				break;

			case sw_first_minor_digit:
				if (ch < '0' || ch > '9') {
					return HTTP_PARSE_INVALID_REQUEST;
				}

				r->http_minor = ch - '0';
				state = sw_minor_digit;
				break;

			case sw_minor_digit:
				if (ch == CR) {
					state = sw_almost_done;
					break;
				}
				
				if (ch == LF) {
					goto done;
				}

				if (ch == ' ') {
					state = sw_spaces_after_digit;
					break;
				}

				if (ch < '0' || ch > '9') {
					return HTTP_PARSE_INVALID_REQUEST;
				}

				r->http_minor = r->http_minor * 10 + ch - '0';
				break;

			case sw_spaces_after_digit:
				switch (ch) {
					case ' ':
						break;
					case CR:
						state = sw_almost_done;
						break;
					case LF:
						goto done;
					default:
						return HTTP_PARSE_INVALID_REQUEST;
				}
				break;

			case sw_almost_done:
				r->request_end = p - 1;
				if (ch == LF) {
					goto done;
				} else {
					return HTTP_PARSE_INVALID_REQUEST;
				}
				break;
		}
	}

	r->pos = pi;
	r->state = state;

	return EAGAIN;

done:
	r->pos = pi + 1;
	if (r->request_end == NULL) {
		r->request_end = p;
	}
	r->state = sw_start;
	return 0;
}
