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

int create_listening_socket(unsigned short port) {
	int listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket < 0) {
		error_handler("socket() failed");
	}

	int enable = 1;
	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&enable, sizeof(enable)) < 0) {
		closesocket(listen_socket);
		error_handler("setsockopt() failed");
	}

	struct sockaddr_in server_addr = build_server_address(port);
	if (bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		closesocket(listen_socket);
		error_handler("bind() failed");
	}

	if (listen(listen_socket, QUEUE_SIZE) < 0) {
		closesocket(listen_socket);
		error_handler("listen() failed");
	}

	return listen_socket;
}

void handle_client(int client_socket) {
	weather_request_t request;
	memset(&request, 0, sizeof(request));

	size_t received_total = 0;
	char *request_bytes = (char *)&request;
	while (received_total < sizeof(request)) {
		int received = recv(client_socket, request_bytes + received_total, (int)(sizeof(request) - received_total), 0);
		if (received <= 0) {
			if (received < 0) {
				perror("recv() failed");
			}
			return;
		}
		received_total += (size_t)received;
	}

	request.city[sizeof(request.city) - 1] = '\0';

	char client_ip[INET_ADDRSTRLEN] = "unknown";
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	if (getpeername(client_socket, (struct sockaddr *)&client_addr, &client_addr_len) == 0) {
		if (inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, sizeof(client_ip)) == NULL) {
			strcpy(client_ip, "unknown");
		}
	}

	printf("Request '%c %s' from client ip %s\n", request.type, request.city, client_ip);

	weather_response_t response;
	memset(&response, 0, sizeof(response));
	response.status = STATUS_INVALID_REQUEST;
	response.type = '\0';
	response.value = 0.0f;

	const int valid_type = (request.type == 't' || request.type == 'h' || request.type == 'w' || request.type == 'p');
	if (!valid_type) {
		response.status = STATUS_INVALID_REQUEST;
	} else if (!is_supported_city(request.city)) {
		response.status = STATUS_CITY_NOT_AVAILABLE;
	} else {
		response.status = STATUS_SUCCESS;
		response.type = request.type;

		switch (request.type) {
			case 't':
				response.value = get_temperature();
				break;
			case 'h':
				response.value = get_humidity();
				break;
			case 'w':
				response.value = get_wind();
				break;
			case 'p':
				response.value = get_pressure();
				break;
			default:
				response.status = STATUS_INVALID_REQUEST;
				response.type = '\0';
				response.value = 0.0f;
				break;
		}
	}

	size_t sent_total = 0;
	const char *response_bytes = (const char *)&response;
	while (sent_total < sizeof(response)) {
		int sent = send(client_socket, response_bytes + sent_total, (int)(sizeof(response) - sent_total), 0);
		if (sent <= 0) {
			if (sent < 0) {
				perror("send() failed");
			}
			return;
		}
		sent_total += (size_t)sent;
	}
}

int main(int argc, char *argv[]) {
#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		fprintf(stderr, "WSAStartup() failed: %d\n", result);
		return EXIT_FAILURE;
	}
#endif

	unsigned short port;
	if (parse_arguments(argc, argv, &port) < 0) {
		fprintf(stderr, "Usage: %s [-p port]\n", argv[0]);
		clearwinsock();
		return EXIT_FAILURE;
	}

	int listen_socket = create_listening_socket(port);
	printf("Weather server listening on port %u\n", port);

	while (1) {
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		int client_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &client_addr_len);
		if (client_socket < 0) {
			perror("accept() failed");
			continue;
		}

		handle_client(client_socket);
		closesocket(client_socket);
	}

	closesocket(listen_socket);
	clearwinsock();
	return EXIT_SUCCESS;
} // main end
