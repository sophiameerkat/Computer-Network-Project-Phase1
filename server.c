#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int ServerFd, ClientFd;
struct sockaddr_in SocketAddress, ClientAddress;
char Content[500000]; 

void SocketSet(int port) {
	if ((ServerFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
		perror("Socket Setup");
	SocketAddress.sin_family = AF_INET;
	SocketAddress.sin_port = htons(port);
	SocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	return;
}

void SocketBind(int ServerFd) {
	int ReuseAddr = 1;
	socklen_t ReuseAddrLen;
	ReuseAddrLen = sizeof(ReuseAddr);
	setsockopt(ServerFd, SOL_SOCKET, SO_REUSEADDR, &ReuseAddr, ReuseAddrLen);
	if (bind(ServerFd, (struct sockaddr *) &SocketAddress, sizeof(SocketAddress)) < 0)
		perror("Socket Bind");
	return;
}

void SocketListen(int ServerFd) {
	if (listen(ServerFd, 10) < 0)
		perror("Listening");
	return;
}

void ReadFile() {
	FILE *f = fopen("index.html", "r");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *string = malloc(fsize + 1);
	fread(string, 1, fsize, f);
	fclose(f);

	string[fsize] = 0;
	strcpy(Content, string);
	return;
}

void SocketAccept(int ServerFd) {
	while (1) {
		int AddrLen = sizeof(struct sockaddr_in);
		if ((ClientFd = accept(ServerFd, (struct sockaddr *)&ClientAddress, (socklen_t*)&AddrLen)) < 0)
			perror("Accept");

		ReadFile();
		char AllFile[8192];

		char ServerReply[6000];
		if (recv(ClientFd, ServerReply , 6000 , 0) < 0)
			perror("Receive");

		sprintf(AllFile, "HTTP/1.1 200 OK\r\nContent-Length: %lu\r\n\r\n %s", strlen(Content), Content);
		
		if (send(ClientFd , AllFile, strlen(AllFile) , 0) < 0)
			perror("Send");

		close(ClientFd);
	}
	return;
}

int main(int argc, char const *argv[]) {
	int port = 80;
	SocketSet(port);
	SocketBind(ServerFd);
	SocketListen(ServerFd);
	SocketAccept(ServerFd);
	return 0;
}
