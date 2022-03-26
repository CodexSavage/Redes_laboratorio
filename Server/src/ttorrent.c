
// Trivial Torrent

// TODO: some includes here

#include "file_io.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>

// TODO: hey!? what is this?

/**
 * This is the magic number (already stored in network byte order).
 * See https://en.wikipedia.org/wiki/Magic_number_(programming)#In_protocols
 */
static const uint32_t MAGIC_NUMBER = 0xde1c3232; // = htonl(0x32321cde);

static const uint8_t MSG_REQUEST = 0;
static const uint8_t MSG_RESPONSE_OK = 1;
static const uint8_t MSG_RESPONSE_NA = 2;

enum { RAW_MESSAGE_SIZE = 13 };
extern int errno;
#define PORT 8080


/**
 * Main function.
 */
int main(int argc, char **argv) {

	set_log_level(LOG_DEBUG);

	log_printf(LOG_INFO, "Trivial Torrent (build %s %s) by %s", __DATE__, __TIME__, "J. DOE and J. DOE");

	// ==========================================================================
	// Parse command line
	// ==========================================================================

	// TODO: some magical lines of code here that call other functions and do various stuff.

	// The following statements most certainly will need to be deleted at some point...
	(void) argc;
	(void) argv;
	(void) MAGIC_NUMBER;
	(void) MSG_REQUEST;
	(void) MSG_RESPONSE_NA;
	(void) MSG_RESPONSE_OK;
	
	struct sockkaddr_in server_struct;
	struct sockaddr_in client_struct;
	char buffer[1024];
	int socket_server = socket(AF_INET,SOCK_STREAM,0);
	if (socket_server < 0){
		perror("ERROR: socket can't be created");
		exit(EXIT_FAILURE);
	}
	printf("[+]TCP server socket created. \n");
	memset(&server, '\0', sizeof(server));
	server_struct.sin_addr = AF_INET;
	server_struct.sin_port = htons(PORT);
	server_struct.sin_addr.s_addr = INADDR_ANY;
	int bind_function = bind(socket_server, (struct sockaddr*)&server_struct, sizeof(server_struct));
	if( bind_function < 0 ){
		print("ERROR MSG: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("[+]Bind to the port number : %d\n",8080);
	listen(socket_server,4);//Función de listen
	printf("Listenning...\n");//Mensaje de confirmación de listen
	while(true){
		socklen_t client_size = sizeof(client_struct); //Entero sin signo de tamaño de 32 bits. Guarda el tamaño de la estrucutura de "client_struct"
		int	socket_client = accept(socket_server,(struct sockaddr*)&client_struct,&client_size);
		if( socket_client < 0){
			printf("ERROR MSG: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		print("[+]Client connected. \n"); //Mensaje de confirmación de acceptación de accept
		/*pid_t pid = fork();
		if ( pid == -1){
			perror("ERROR: fork can't be created");
			exit(EXIT_FAILURE);
		}*/
		size_t bufflen = sizeof(buffer); //tamaño del buffer
		//ssize_t recv(int sockfd, void *buf, size_t len, int flags);		
		ssize_t re = recv(socket_client, buffer, bufflen,0);
	}

	return 0;
}
