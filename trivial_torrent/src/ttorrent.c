
// Trivial Torrent

// TODO: some includes here

#include "file_io.h"
#include "logger.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>

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
//const uint8_t PUERTO = 8080;

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

	struct torrent_t torrent;
	struct sockaddr_in server;
	if ( create_torrent_from_metainfo_file("file.torrrent", (struct torrent_t*)&torrent, "file")){
		//handle error
	}
	if (torrent.block_map[0]){
		//block 0 is avaible
		struct block_t block;
		if ( load_block(&torrent,0,&block)){
			//Almacena un bloque en el archivo descargado
			//habdle error
		}

		//do stuff with this block... --> hacer cosas con este bloque

	}
	struct block_t another_block;
	// fill this block somehow...--> llene este bloque de alguna manera
	int r = store_block(&torrent,1,&another_block);
	//desasigna todos los campos necesarios en una estructura torrent_t. También cierra el arroyo downloaded_files_stream
	if (r){
		printf("ERROR: has been ocurred %s\n",strerror(errno));
		exit(1);
		//block was not stored correctly, either because of an invalid has pr because of an I/O error.
		//El bloque no se almacenó correctamente, ya sea debido a un pr no válido, debido a un error de E/S.
	} else {
		// the block was correctly stored and torrent.block_map[1] evaluates now to true.
		// El bloque fue correctamente almacenado y torrent.block_map[1] evaluado ahora como verdadero
		assert(torrent.block_map[1]);
	}
	if(destroy_torrent(&torrent)){
		// error handling --> manejo de error
	}

	char buffer[64000];

	int socketClient = socket( AF_INET, SOCK_STREAM,0);
	if (socketClient < 0){
	  printf("ERROR: %s\n", strerror(errno));
	  exit(1);
	}
	printf("[+]Socket client is created \n");
	memset(&server, '\0', sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(8080);
	server.sin_addr.s_addr = INADDR_ANY;
	int conn = connect(socketClient, ( struct sockaddr*)&server, sizeof(server));
	if ( conn < 0 ){
	  printf("ERROR: %s\n",strerror(errno));
	  exit(1);
	}
	printf("[+]Connection succes");
	size_t len_buffer = sizeof(buffer);
	while(1){
	  ssize_t sen = send(socketClient, buffer, len_buffer,0);
	  if ( sen < 0 ){
	    printf("ERROR:Disconnection has been ocurred");
	    exit(1);
	  }
	  ssize_t rec = recv(socketClient, buffer, len_buffer,0);
	  if ( rec < 0 ){
  	  printf("Error: %s\n", strerror(errno));
		  exit(1);
	  }
	}
	close(socketClient);
	return 0;
}