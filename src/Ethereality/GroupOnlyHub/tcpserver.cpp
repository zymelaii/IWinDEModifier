#include "tcpserver.h"

#define DEFAULT_BACKLOG 128

static uv_loop_t*		  loop;
static struct sockaddr_in addr;

void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
	buf->base = (char*)malloc(suggested_size);
	buf->len  = suggested_size;
}

void on_close(uv_handle_t* handle) {
	free(handle);
}

void on_write(uv_write_t* req, int status) {
	if (status) {
		fprintf(stderr, "Write error %s\n", uv_strerror(status));
	}
	write_req_t* wr = (write_req_t*)req;
	free(wr->buf.base);
	free(wr);
}

void on_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf) {
	if (nread > 0) {
		char reply[256];
		if (strcmp(buf->base, "/reply") == 0) {
			strcpy_s(reply, "Hello World!");
		} else {
			strcpy_s(reply, "ERROR");
		}

		write_req_t* req = (write_req_t*)malloc(sizeof(write_req_t));
		req->buf		 = uv_buf_init(_strdup(reply), strlen(reply) + 1);

		printf("<-- recv: \"%.*s\"\n--> send: \"%.*s\"\n",
			   (int)buf->len,
			   buf->base,
			   (int)req->buf.len,
			   req->buf.base);

		uv_write((uv_write_t*)req, client, &req->buf, 1, on_write);

		return;
	}

	if (nread < 0) {
		if (nread != UV_EOF) fprintf(stderr, "Read error %s\n", uv_err_name(nread));
		uv_close((uv_handle_t*)client, on_close);
	}

	free(buf->base);
}

void on_new_connection(uv_stream_t* server, int status) {
	if (status < 0) return;

	uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	uv_tcp_init(loop, client);
	if (uv_accept(server, (uv_stream_t*)client) == 0) {
		uv_read_start((uv_stream_t*)client, on_alloc, on_read);
	} else {
		uv_close((uv_handle_t*)client, on_close);
	}
}

int startup(const char* ip, int port) {
	loop = uv_default_loop();

	uv_tcp_t server;
	uv_tcp_init(loop, &server);

	uv_ip4_addr(ip, port, &addr);
	uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);

	int err = uv_listen((uv_stream_t*)&server, DEFAULT_BACKLOG, on_new_connection);
	if (err) {
		fprintf(stderr, "Listen error %s\n", uv_strerror(err));
		return 1;
	}

	return uv_run(loop, UV_RUN_DEFAULT);
}