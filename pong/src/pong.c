/**
 * This file implements a "UDP ping server".
 *
 * It basically waits for datagrams to arrive, and for each one received, it responds to the original sender
 * with another datagram that has the same payload as the original datagram. The server must reply to 3
 * datagrams and then quit.
 *
 * Test with:
 *
 * $ netcat -u 127.0.0.1 8080
 * ping
 * ping
 * pong
 * pong
 *
 * (assuming that this program listens at localhost port 8080)
 *
 */

// TODO: some includes here
#include<sys/socket.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<stdlib.h>
#include<stdio.h>
#include<netinet/in.h>
#include "logger.h"
#include<string.h>
#include<arpa/inet.h>
#include<netdb.h>
int main(int argc, char **argv) {

	(void) argc; // This is how an unused parameter warning is silenced.
	(void) argv;
	struct sockaddr_in server;
	struct sockaddr_in client = { 0 };
	//char *ip = "127.0.0.1";
	socklen_t client_len = sizeof(client);
	//uint16_t portno = atoi(argv[2]);
	//socket(int domain,int type, int protocol );
	//int recv, sen;
	//char buff[1024];
	//configuracion del servidor
	memset(&server, '\0', sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(8080);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	//server.sin_addr.s_addr = inet_addr("127.0.0.1");
	//socket
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock == -1){
		perror("Error: Socket not be created");
		exit(1);
	}
	//socklen_t serverlen = sizeof(server);
	// bind (sock, const struct sockaddr * address, socklen_t address_len);
	int bin = bind(sock, (struct sockaddr*) &server, sizeof(server));
	if (bin < 0) {
		perror("Error en la implementacion del bind ");
		exit(1);
	}
	//memset(&client,0,sizeof(client));
	char buffer[64000];
	size_t bufflen = sizeof(buffer);
	for ( int i = 0; i < 3; i ++){
		//recvfrom()
		ssize_t recv = recvfrom(sock, buffer, bufflen, 0, (struct sockaddr *)&client, &client_len);
		if ( recv < 0){
			perror("Error recive a message");
			exit(1);
		}/*
		if (recv == 0){
			printf("No messages found");
		}*/
		ssize_t sen = sendto(sock, buffer, (size_t)recv, 0, (struct sockaddr *)&client, client_len);
		if( sen < 0){
			perror("Error in sendto");
			exit(1);
		}
	}
	// TODO: some socket stuff here
	
	return 0;
}
