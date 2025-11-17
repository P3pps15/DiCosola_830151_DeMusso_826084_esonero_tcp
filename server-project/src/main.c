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
#include <string.h>
#include <time.h>
#include "protocol.h"

#if defined WIN32
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#endif

#define NO_ERROR 0
#define SEED_RNG_ONCE() \
	do { \
		static int seeded = 0; \
		if (!seeded) { \
			srand((unsigned int)time(NULL)); \
			seeded = 1; \
		} \
	} while (0)

static const char *SUPPORTED_CITIES[] = {
	"Bari",
	"Roma",
	"Milano",
	"Napoli",
	"Torino",
	"Palermo",
	"Genova",
	"Bologna",
	"Firenze",
	"Venezia"
};

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

float get_humidity(void) {
	const float min = 20.0f;
	const float max = 100.0f;
	SEED_RNG_ONCE();
	float normalized = (float)rand() / (float)RAND_MAX;
	return min + normalized * (max - min);
}

float get_wind(void) {
	const float min = 0.0f;
	const float max = 100.0f;
	SEED_RNG_ONCE();
	float normalized = (float)rand() / (float)RAND_MAX;
	return min + normalized * (max - min);
}

float get_pressure(void) {
	const float min = 950.0f;
	const float max = 1050.0f;
	SEED_RNG_ONCE();
	float normalized = (float)rand() / (float)RAND_MAX;
	return min + normalized * (max - min);
}

int is_supported_city(const char *city) {
	if (city == NULL) {
		return 0;
	}

	const size_t total = sizeof(SUPPORTED_CITIES) / sizeof(SUPPORTED_CITIES[0]);
	for (size_t i = 0; i < total; ++i) {
		if (strcasecmp(city, SUPPORTED_CITIES[i]) == 0) {
			return 1;
		}
	}

	return 0;
}

int parse_arguments(int argc, char *argv[], unsigned short *port) {
	if (port == NULL) {
		return -1;
	}

	*port = DEFAULT_SERVER_PORT;

	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-p") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "Missing value for -p option.\n");
				return -1;
			}

			char *endptr = NULL;
			long value = strtol(argv[++i], &endptr, 10);
			if (endptr == NULL || *endptr != '\0') {
				fprintf(stderr, "Invalid port value: %s\n", argv[i]);
				return -1;
			}

			if (value <= 0 || value > 65535) {
				fprintf(stderr, "Port must be in range 1-65535.\n");
				return -1;
			}

			*port = (unsigned short)value;
		} else {
			fprintf(stderr, "Unknown argument: %s\n", argv[i]);
			return -1;
		}
	}

	return 0;
}

struct sockaddr_in build_server_address(unsigned short port) {
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	return server_addr;
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
