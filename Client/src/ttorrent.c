// Trivial Torrent
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
#include <inttypes.h>
#include <string.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <stdbool.h> // booleanos
#include <fcntl.h>

// TODO: hey!? what is this?

/**
 * This is the magic number (already stored in network byte order).
 * See https://en.wikipedia.org/wiki/Magic_number_(programming)#In_protocols
 */
static const uint32_t MAGIC_NUMBER = 0xde1c3232; // = htonl(0x32321cde);

static const uint8_t MSG_REQUEST = 0;
static const uint8_t MSG_RESPONSE_OK = 1;
static const uint8_t MSG_RESPONSE_NA = 2;

enum
{
	RAW_MESSAGE_SIZE = 13
};

static void inicializar_buffer(uint8_t buffer[RAW_MESSAGE_SIZE])
{
	uint32_t variable = htonl(MAGIC_NUMBER);
	buffer[0] = (uint8_t) variable; // casting de parametro MAGIC_NUMBER
	for (int k = 1; k < 4; k++)
	{
		variable = (variable >> 8);
		buffer[k] = (uint8_t)variable;
	}
	buffer[4] = MSG_REQUEST;
}


void log_buffer(const char* label, const unsigned char* buff, const size_t len);

void log_buffer(const char* label, const unsigned char* buff, const size_t len)
{
	assert(label != NULL);

	fprintf(stderr, "%s: ", label);
	for (size_t i = 0; i < len; i++)
	{
		fprintf(stderr, "%02X ", buff[i]);
		if (i > 0 && i % 16 == 0)
			fprintf(stderr, "\n");
	}
		
	fprintf(stderr, "\n");
}

int cliente(char *metainfo_file_name);

int cliente(char *metainfo_file_name)
{
	struct torrent_t torrent; // estrucutra torrent para el archivo ".ttorrent"
	struct block_t block;	  // estructura block para los bloques

	size_t metainfo_strlen = strlen(metainfo_file_name);
	char *downloaded_file_name = malloc(metainfo_strlen + 1);

	memset(downloaded_file_name, 0, metainfo_strlen);
	memcpy(downloaded_file_name, metainfo_file_name, metainfo_strlen);

	for (size_t i = 0; i < 10; i++)
	{
		downloaded_file_name[metainfo_strlen - i] = 0;
	}

	int create = create_torrent_from_metainfo_file(metainfo_file_name, &torrent, downloaded_file_name); // Create
	if (create < 0)																						// Error handling...
	{
		printf("Error: %s\n", strerror(errno));
		exit(0);
	}

	struct sockaddr_in server; // estructura server para la definición de conexiones

	uint8_t buffer[RAW_MESSAGE_SIZE];
	uint8_t bufferRecv[RAW_MESSAGE_SIZE];
	uint64_t all_blocks = 0;

	for (uint64_t i = 0; i < torrent.peer_count; i++)
	{
		int socketClient = socket(AF_INET, SOCK_STREAM, 0); // Socket creation

		if (socketClient < 0) // Error handling...
		{
			printf("Error: %s\n", strerror(errno));
			return 1;
		}

		log_printf(LOG_INFO, "[+]socket is created successfully \n");

		log_printf(LOG_INFO, "%" PRIu64 "\n", all_blocks);
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = inet_addr("127.0.0.1");
		server.sin_port = torrent.peers[i].peer_port;
		memcpy(&server.sin_addr.s_addr, &torrent.peers[i].peer_address, 4);

		int conn = connect(socketClient, (struct sockaddr *)&server, (socklen_t)sizeof(server)); // Connection
		if (conn < 0)																			 // Error handling...
		{
			close(socketClient);
			printf("ERROR CONNECT:connect %s\n______", strerror(errno));
			continue;
		}

		log_printf(LOG_INFO, "___CONNECT:Connection succes____________");
		for (uint64_t l = 0; l < torrent.block_count; l++)
		{
			if (torrent.block_map[l])
			{
				continue;
			}
		
			inicializar_buffer(buffer);

			uint64_t v = l;
			for (int m = RAW_MESSAGE_SIZE - 1; m >= 6; m--)
			{
				buffer[m] = (uint8_t) (v & 0xff);
				v = (v >> 8);
			}


			log_buffer("El buffer que enviamos", buffer, sizeof(buffer));

			sleep(1);

			ssize_t sen = send(socketClient, buffer, sizeof(buffer), 0); // Send function
			if (sen < 0)												 // Error handling...
			{
				printf("ERROR: send %s \n", strerror(errno));
				exit(0);
			}


			printf("SEND: Mensaje enviado %lu\n", sen);
			ssize_t re = recv(socketClient, bufferRecv, sizeof(bufferRecv), MSG_WAITALL);
			printf("recibimos %ld\n", re);
			if (re < 0)
			{
				printf("Error: recv %s \n", strerror(errno));
				exit(0);
			}

			if (re == 0)
			{
				printf("Error: el servidor se ha desconectado \n");
				break;
			}

			// bufferRecv
			uint64_t variable = bufferRecv[RAW_MESSAGE_SIZE - 1];
			for (int t = (RAW_MESSAGE_SIZE - 2); t >= 5; t--)
			{
				variable = (variable << 8);
				variable |= (uint32_t)bufferRecv[t];
			}

			printf("Recibimos bloque %lu\n", l);
			log_buffer("El buffer que recibimos", bufferRecv, sizeof(bufferRecv));

			if (bufferRecv[4] == MSG_RESPONSE_OK)
			{
				block.size = get_block_size(&torrent, l);
				ssize_t rec = recv(socketClient, block.data, block.size, MSG_WAITALL);
				if (rec < 0)
				{
					printf("ERROR: recv %s \n", strerror(errno));
					log_printf(LOG_INFO, "Problem has been ocurred in recv");
					exit(0);
				}
				else
				{
					int r = store_block(&torrent, l, &block);
					if (r < 0) //(r == -1) <-- error handling
					{
						printf("ERROR: STORE_BLOCK %s \n", strerror(errno));
						log_printf(LOG_INFO, "Can`t store block");
						exit(0);
					}
					else
					{
						printf("L vale %lu\n", l);
						assert(torrent.block_map[l]);
						log_printf(LOG_INFO, "[+]Store block");
						torrent.block_map[l] = 1;
						all_blocks++;
					}
				}
			}
			else if (bufferRecv[4] == MSG_RESPONSE_NA)
				log_printf(LOG_INFO, "Not available");
		}

		if (close(socketClient) < 0)
		{
			perror("ERROR: no hem pogut tancar el socket\n");
			return 1;
		}
	}

	if (destroy_torrent(&torrent) < 0) // Error handling...
	{
		printf("ERROR: fail destroy torrent %s\n", strerror(errno));
		log_printf(LOG_INFO, "Problema has been ocurred during destroy torrent");
		exit(0);
	}
	log_printf(LOG_INFO, "[+]Destroy torrent");

	return 0;
}


void server(char* port_str, char *metainfo_file_name);

void server(char* port_str, char *metainfo_file_name)
{
	struct torrent_t torrent;
	struct sockaddr_in Server;
	struct sockaddr_in Client;
	//
	memset(&Server, 0, sizeof(Server));
	memset(&Client, 0, sizeof(Client));
	//
	struct block_t block;
	struct pollfd fds[10];
	uint8_t buffer[RAW_MESSAGE_SIZE];
	uint8_t bufferRecv[RAW_MESSAGE_SIZE];
	uint64_t blockRecv_;
	bool con = false;
	// const char download_file[] = "test_file";

	size_t metainfo_strlen = strlen(metainfo_file_name);
	char *downloaded_file_name = malloc(metainfo_strlen + 1);

	memset(downloaded_file_name, 0, metainfo_strlen);
	memcpy(downloaded_file_name, metainfo_file_name, metainfo_strlen);

	for (size_t i = 0; i < 10; i++)
	{
		downloaded_file_name[metainfo_strlen - i] = 0;
	}

	ssize_t send1;
	int create = create_torrent_from_metainfo_file(metainfo_file_name, &torrent, downloaded_file_name);
	if (create < 0)
	{
		printf("ERROR: create %s \n", strerror(errno));
		exit(1);
	}

	Server.sin_family = AF_INET;
	Server.sin_port = htons((uint16_t)atoi(port_str));
	Server.sin_addr.s_addr = htonl(INADDR_ANY);
	// Server.sin_addr.s_addr = htonl(INADDR_ANY);

	int socketServer = socket(AF_INET, SOCK_STREAM, 0);
	if (socketServer < 0)
	{
		printf("ERROR: Socket %s \n", strerror(errno));
		exit(0);
	}

	log_printf(LOG_INFO, "\n[+]Socket create succesfully");

	int flags = fcntl(socketServer, F_GETFL);
  	int ioc = fcntl(socketServer, F_SETFL, flags | O_NONBLOCK);
	if (ioc < 0)
	{
		printf("ERROR: ioctl %s \n", strerror(errno));
		log_printf(LOG_INFO, "Problema has been ocurred in fcntl(disable non-blocking)");
		exit(0);
	}

	if (bind(socketServer, (struct sockaddr *)&Server, sizeof(Server)) < 0)
	{
		printf("ERROR: BIND %s \n", strerror(errno));
		exit(0);
	}

	log_printf(LOG_INFO, "\n[+]Bind correct");
	if (listen(socketServer, 10) < 0)
	{
		printf("ERROR: LISTEN %s \n", strerror(errno));
		close(socketServer);
		exit(0);
	}

	log_printf(LOG_INFO, "\n[+]Listen correct");
	nfds_t nfds = 1;

	memset(fds, 0, sizeof(fds));

	fds[0].fd = socketServer;
	fds[0].events = POLLIN;

	while (true)
	{
		ssize_t pol = poll(fds, nfds, -1);
		if (pol < 0)
		{
			printf("ERROR: poll %s \n", strerror(errno));
			exit(0);
		}


		log_printf(LOG_INFO, "\n[+]Poll correct");

		for (int i = 1; i < 10; i++)
		{
			if (fds[i].fd == 0)
				continue;

			if (fds[i].revents != 0 && fds[i].revents != POLLIN)
			{
				printf("ERROR: fds.revents %s \n", strerror(errno));
				exit(0);
			}

			ssize_t rec = recv(fds[i].fd, bufferRecv, RAW_MESSAGE_SIZE, 0);
			if (rec < 0)
			{
				printf("ERROR: recv %s \n", strerror(errno));
				exit(0);
			}
			else if (!rec)
			{
				log_printf(LOG_INFO, "[+]Terminate");
				con = true;
			}
			else
			{
				printf("aaaaa\n");
				log_buffer("El buffer que recibimos", bufferRecv, sizeof(bufferRecv));
				if (bufferRecv[4] == MSG_REQUEST)
				{
					uint64_t variable = 0;
					for (int t = 5; t < RAW_MESSAGE_SIZE; t++)
					{
						printf("variable %lu\n", variable);
						variable = (variable << 8);
						variable |= (uint32_t)bufferRecv[t];
					}

					printf("variable final %lu\n", variable);

					blockRecv_ = variable;
					for (int o = 0; o < RAW_MESSAGE_SIZE; o++)
					{
						buffer[o] = bufferRecv[o];
					}

					printf("bbbbbbb variable = %lu\n", variable);

					if (torrent.block_map[blockRecv_]) // definir blockRecv en variable
					{
						printf("el if\n");
						int load = load_block(&torrent, blockRecv_, &block);
						if (load < 0)
						{
							printf("Error: load_block %s \n", strerror(errno));
							exit(0);
						}
						else
						{
							buffer[4] = MSG_RESPONSE_OK;

							log_buffer("El buffer que enviamos", buffer, RAW_MESSAGE_SIZE);
							send1 = send(fds[i].fd, buffer, RAW_MESSAGE_SIZE, 0);
							assert(send1 == (ssize_t)RAW_MESSAGE_SIZE);

							if (send1 < 0)
							{
								printf("Error: send function %s \n", strerror(errno));
								exit(0);
							}

							printf("block size = %lu\n", block.size);
							send1 = send(fds[i].fd, block.data, block.size, 0);
							assert((uint64_t)send1 == block.size);
						}
					}
					else
					{
						printf("el else\n");
						buffer[4] = MSG_RESPONSE_NA;
						ssize_t send2 = send(fds[i].fd, buffer, RAW_MESSAGE_SIZE, MSG_EOR);
						if (send2 < 0)
						{
							printf("ERROR: send3 %s \n", strerror(errno));
							log_printf(LOG_INFO, "Problem has been ocurred in send3");
							exit(0);
						}
						log_printf(LOG_INFO, "\nBloque no disponible");
					}
				}
				else
				{
					perror("Incorrect");
					exit(0);
				}
			}

			if (con == true)
			{
				printf("Cierra la conexión\n");
				if (close(fds[i].fd) < 0)
				{
					printf("ERROR: close fds %s \n", strerror(errno));
					log_printf(LOG_INFO, "Problem has been ocurred in clo");
					exit(EXIT_FAILURE);
				}
				nfds--;
				fds[i].fd = -1;
			}
		}

		if (fds[0].revents == POLLIN)
		{
			int socketA = accept(socketServer, NULL, NULL);
			if (socketA < 0)
			{
				printf("ERROR: accept socket %s \n", strerror(errno));
				exit(0);
			}
			else if (socketA > 0)
			{
				int flagsA = fcntl(socketA, F_GETFL);
  				fcntl(socketA, F_SETFL, flagsA | O_NONBLOCK);

				log_printf(LOG_INFO, "[+]Conexion aceptada");
				fds[nfds].events = POLLIN;
				fds[nfds].fd = socketA;
				nfds++;
			}
		}
	}
	if (close(socketServer) < 0)
	{
		printf("ERROR: closefd %s \n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

/**
 * Main function.
 */
int main(int argc, char **argv)
{

	set_log_level(LOG_DEBUG);

	log_printf(LOG_INFO, "Trivial Torrent (build %s %s) by %s", __DATE__, __TIME__, "J. DOE and J. DOE");

	// ==========================================================================
	// Parse command line
	// ==========================================================================

	// TODO: some magical lines of code here that call other functions and do various stuff.

	// The following statements most certainly will need to be deleted at some point...

	if (argc < 2)
	{
		printf("Error: no hay suficientes argumentos.\n");
		exit(0);
	}

	//----------------------------------------------------------------------------
	//				PARTE CLIENTE
	//----------------------------------------------------------------------------
	if (argc < 3)
	{
		cliente(argv[1]);
	}
	//--------------------------------------------------------------------------
	//			PARTE SERVIDOR
	//--------------------------------------------------------------------------
	else
	{
		server(argv[2], argv[3]);
	}
	return 0;
}
