#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

typedef struct {
	uv_write_t req;
	uv_buf_t   buf;
} write_req_t;

int startup(const char* ip, int port);