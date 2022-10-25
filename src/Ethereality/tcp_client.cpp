#include <winsock2.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	WORD	wVersionRequested;
	WSADATA wsaData;
	int		err;
	wVersionRequested = MAKEWORD(1, 1);
	err				  = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return 1;
	}
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return 1;
	}

	SOCKET		sockClient = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	addrSrv.sin_family			 = AF_INET;
	addrSrv.sin_port			 = htons(7000);	  //设置端口号

	char buf[16];

	connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	send(sockClient, "lisi", strlen("lisi") + 1, 0);
	recv(sockClient, buf, 16, 0);
	puts(buf);
	send(sockClient, "si", strlen("si") + 1, 0);
	recv(sockClient, buf, 16, 0);
	puts(buf);
	send(sockClient, "sicc", strlen("sicc") + 1, 0);
	recv(sockClient, buf, 16, 0);
	puts(buf);
	closesocket(sockClient);
	WSACleanup();

	return 0;
}