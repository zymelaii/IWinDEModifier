#include <winsock2.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	WORD	wVersionRequested;
	WSADATA wsaData;
	int		err;
	wVersionRequested = MAKEWORD(1, 1);
	err				  = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) return 1;
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return 1;
	}

	const char* ip		= "127.0.0.1";
	int			port	= 8080;
	const char* text	= "Hello World!";
	int			no_send = 0;
	int			no_recv = 0;
	for (int i = 1; i < argc; ++i) {
		if (!argv[i][0]) continue;
		if (strcmp(argv[i], "--ip") == 0 && i + 1 < argc) {
			ip		   = strdup(argv[i + 1]);
			argv[i][0] = argv[i + 1][0] = 0;
			break;
		}
	}
	for (int i = 1; i < argc; ++i) {
		if (!argv[i][0]) continue;
		if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
			port	   = atoi(argv[i + 1]);
			argv[i][0] = argv[i + 1][0] = 0;
			break;
		}
	}
	for (int i = 1; i < argc; ++i) {
		if (!argv[i][0]) continue;
		if (strcmp(argv[i], "--no-send") == 0) {
			no_send	   = 1;
			argv[i][0] = 0;
			break;
		}
	}
	for (int i = 1; i < argc; ++i) {
		if (!argv[i][0]) continue;
		if (strcmp(argv[i], "--no-recv") == 0) {
			no_recv	   = 1;
			argv[i][0] = 0;
			break;
		}
	}
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] != '\0') {
			text = argv[i];
			break;
		}
	}

	SOCKET		sockClient = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr(ip);
	addrSrv.sin_family			 = AF_INET;
	addrSrv.sin_port			 = htons(port);

	printf("post: \"%s:%d\" \"%s\"\n", ip, port, text);

	if (connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) != 0) {
		fprintf(stderr, "error: %#x", WSAGetLastError());
		SOCKET_ERROR;
		return 0;
	}

	for (int i = 0; i < 65536; ++i) {
		char buf[1024];
		if (!no_send) send(sockClient, text, strlen(text) + 1, 0);
		if (!no_recv) recv(sockClient, buf, sizeof(buf), 0);
	}

	closesocket(sockClient);
	WSACleanup();

	return 0;
}