/*
 * main.c
 *
 * TCP Server - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP server
 * portable across Windows, Linux and macOS.
 */

#if defined WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "protocol.h"

#define NO_ERROR 0
#define SEED_RNG_ONCE() \
	do { \
		static int seeded = 0; \
		if (!seeded) { \
			srand((unsigned int)time(NULL)); \
			seeded = 1; \
		} \
	} while (0)

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

void error_handler(const char *message) {
	perror(message);
	clearwinsock();
	exit(EXIT_FAILURE);
}

float get_temperature(void) {
	const float min = -10.0f;
	const float max = 40.0f;
	SEED_RNG_ONCE();
	float normalized = (float)rand() / (float)RAND_MAX;
	return min + normalized * (max - min);
}

int main(int argc, char *argv[]) {

	// TODO: Implement server logic

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif

	int my_socket;

	// Create TCP socket
	my_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (my_socket < 0) {
		error_handler("socket() failed");
	}

	// TODO: Configure server address
	// struct sockaddr_in server_addr;
	// server_addr.sin_family = AF_INET;
	// server_addr.sin_port = htons(SERVER_PORT);
	// server_addr.sin_addr.s_addr = INADDR_ANY;

	// TODO: Bind socket
	// bind(...);

	// TODO: Set socket to listen
	// listen(...);

	// TODO: Implement connection acceptance loop
	// while (1) {
	//     int client_socket = accept(...);
	//     // Handle client communication
	//     closesocket(client_socket);
	// }

	printf("Server terminated.\n");

	closesocket(my_socket);
	clearwinsock();
	return 0;
} // main end
