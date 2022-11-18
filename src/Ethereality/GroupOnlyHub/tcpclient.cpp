#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <uv.h>

#include "GroupOnlyHub.h"

struct client_t {
	uv_connect_t  conn;
	GroupOnlyHub* hub;
};

void on_connect(uv_connect_t* conn, int status) {
	if (status < 0) return;
	GroupOnlyHub* hub = ((client_t*)conn)->hub;

	typedef struct {
		uv_write_t req;
		uv_buf_t   buf;
	} write_req_t;

	while (1) {
		auto		 req   = hub->getRequest();
		char*		 s_req = _strdup(req.has_value() ? req->c_str() : "/pull");
		write_req_t* wr	   = (write_req_t*)malloc(sizeof(write_req_t));
		wr->buf			   = uv_buf_init(s_req, strlen(s_req) + 1);

		uv_write((uv_write_t*)wr, conn->handle, &wr->buf, 1, [](uv_write_t* req, int status) {
			write_req_t* wr = (write_req_t*)req;
			free(wr->buf.base);
			free(wr);
			uv_read_start((uv_stream_t*)req->handle,
						  [](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
							  buf->base = (char*)malloc(suggested_size);
							  buf->len	= suggested_size;
						  },
						  [](uv_stream_t* client, ssize_t nread, const uv_buf_t* buf) {
							  printf("<-- recv: \"%.*s\"\n", (int)buf->len, buf->base);
							  free(buf->base);
							  uv_close((uv_handle_t*)client,
									   [](uv_handle_t* handle) { free(handle); });
						  });
		});

		if (!req.has_value()) break;
	}

	free(conn);
}

int startup(const char* ip, int port, GroupOnlyHub* hub) {
	static uv_tcp_t client;
	uv_tcp_init(uv_default_loop(), &client);
	uv_connect_t* conn	   = (uv_connect_t*)malloc(sizeof(client_t));
	((client_t*)conn)->hub = hub;
	struct sockaddr_in addr;
	uv_ip4_addr(ip, port, &addr);
	int err = uv_tcp_connect(conn, &client, (const struct sockaddr*)&addr, on_connect);
	if (err) {
		free(conn);
		return 1;
	}
	return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

int main(int argc, char* argv[]) {
	static GroupOnlyHub hub;
	return startup("0.0.0.0", 5379, &hub);
}