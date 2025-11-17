/*
 * main.c
 *
 * TCP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP client
 * portable across Windows, Linux and macOS.
 */

#if defined(_WIN32) || defined(WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NO_ERROR
#define NO_ERROR 0
#endif
#include "protocol.h"

void clearwinsock() {
#if defined(_WIN32) || defined(WIN32)
	WSACleanup();
#endif
}

void error_handler(const char *message) {
	perror(message);
	clearwinsock();
	exit(EXIT_FAILURE);
}

int parse_request(const char *request_arg, weather_request_t *out_request) {
	if (request_arg == NULL || out_request == NULL) {
		return -1;
	}

	// Skip leading whitespace
	while (*request_arg == ' ' || *request_arg == '\t') {
		++request_arg;
	}

	if (*request_arg == '\0') {
		return -1;
	}

	out_request->type = (char) tolower((unsigned char) *request_arg);

	const char *city_start = request_arg + 1;
	while (*city_start == ' ' || *city_start == '\t') {
		++city_start;
	}

	if (*city_start == '\0') {
		return -1;
	}

	strncpy(out_request->city, city_start, MAX_CITY_LEN - 1);
	out_request->city[MAX_CITY_LEN - 1] = '\0';

	// Trim trailing whitespace from city
	size_t len = strlen(out_request->city);
	while (len > 0) {
		char last_char = out_request->city[len - 1];
		if (last_char == ' ' || last_char == '\t' || last_char == '\n' || last_char == '\r') {
			out_request->city[len - 1] = '\0';
			--len;
		} else {
			break;
		}
	}

	return 0;
}

int connect_to_server(const char *server_address, unsigned short port) {
	const char *address = server_address != NULL ? server_address : DEFAULT_SERVER_ADDRESS;

	int client_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_socket < 0) {
		return -1;
	}

	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_port = htons(port);

	int resolved = 0;
#if defined(_WIN32) || defined(WIN32)
	unsigned long addr_numeric = inet_addr(address);
	if (addr_numeric != INADDR_NONE) {
		sad.sin_addr.s_addr = addr_numeric;
		resolved = 1;
	}
#else
	if (inet_pton(AF_INET, address, &sad.sin_addr) > 0) {
		resolved = 1;
	}
#endif

	if (!resolved) {
		struct hostent *host = gethostbyname(address);
		if (host == NULL || host->h_addr_list == NULL || host->h_addr_list[0] == NULL) {
			closesocket(client_socket);
			return -1;
		}
		memcpy(&sad.sin_addr, host->h_addr_list[0], (size_t) host->h_length);
	}

	if (connect(client_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		closesocket(client_socket);
		return -1;
	}

	return client_socket;
}

int send_weather_request(int socket_fd, const weather_request_t *request) {
	if (socket_fd < 0 || request == NULL) {
		return -1;
	}

	const char *buffer = (const char*) request;
	const size_t total_size = sizeof(weather_request_t);
	size_t sent_bytes = 0;

	while (sent_bytes < total_size) {
		int result = send(socket_fd, buffer + sent_bytes, (int) (total_size - sent_bytes), 0);
		if (result <= 0) {
			return -1;
		}
		sent_bytes += (size_t) result;
	}

	return 0;
}

int receive_weather_response(int socket_fd, weather_response_t *response) {
	if (socket_fd < 0 || response == NULL) {
		return -1;
	}

	char *buffer = (char*) response;
	const size_t total_size = sizeof(weather_response_t);
	size_t received_bytes = 0;

	while (received_bytes < total_size) {
		int result = recv(socket_fd, buffer + received_bytes, (int) (total_size - received_bytes), 0);
		if (result <= 0) {
			return -1;
		}
		received_bytes += (size_t) result;
	}

	return 0;
}

int format_response_message(const weather_response_t *response,
		const weather_request_t *request,
		const char *server_ip,
		char *out_buffer,
		size_t out_size) {
	if (response == NULL || server_ip == NULL || out_buffer == NULL || out_size == 0) {
		return -1;
	}

	char city_label[MAX_CITY_LEN];
	memset(city_label, 0, sizeof(city_label));

	if (request != NULL && request->city[0] != '\0') {
		strncpy(city_label, request->city, MAX_CITY_LEN - 1);
		size_t len = strlen(city_label);
		for (size_t i = 0; i < len; ++i) {
			if (i == 0) {
				city_label[i] = (char) toupper((unsigned char) city_label[i]);
			} else {
				city_label[i] = (char) tolower((unsigned char) city_label[i]);
			}
		}
	}

	char payload[RESPONSE_MESSAGE_LEN];
	memset(payload, 0, sizeof(payload));

	if (response->status == STATUS_SUCCESS) {
		const char type = (char) tolower((unsigned char) response->type);
		const char *city_output = city_label[0] != '\0' ? city_label : "Città";

		switch (type) {
		case 't':
			snprintf(payload, sizeof(payload), "%s: Temperatura = %.1f°C", city_output, response->value);
			break;
		case 'h':
			snprintf(payload, sizeof(payload), "%s: Umidità = %.1f%%", city_output, response->value);
			break;
		case 'w':
			snprintf(payload, sizeof(payload), "%s: Vento = %.1f km/h", city_output, response->value);
			break;
		case 'p':
			snprintf(payload, sizeof(payload), "%s: Pressione = %.1f hPa", city_output, response->value);
			break;
		default:
			return -1;
		}
	} else if (response->status == STATUS_CITY_NOT_AVAILABLE) {
		snprintf(payload, sizeof(payload), "Città non disponibile");
	} else if (response->status == STATUS_INVALID_REQUEST) {
		snprintf(payload, sizeof(payload), "Richiesta non valida");
	} else {
		snprintf(payload, sizeof(payload), "Risposta non valida");
	}

	int written = snprintf(out_buffer, out_size, "Ricevuto risultato dal server ip %s. %s", server_ip, payload);
	if (written < 0 || (size_t) written >= out_size) {
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[]) {
	int exit_code = EXIT_FAILURE;
	int client_socket = -1;
	weather_request_t request;
	weather_response_t response;
	char response_message[RESPONSE_MESSAGE_LEN];
	char server_ip[INET_ADDRSTRLEN] = {0};
	char server_address[BUFFER_SIZE];
	unsigned short server_port = DEFAULT_SERVER_PORT;
	const char usage_format[] = "Usage: %s [-s server] [-p port] -r \"type city\"\n";

	memset(&request, 0, sizeof(request));
	memset(&response, 0, sizeof(response));
	memset(response_message, 0, sizeof(response_message));
	memset(server_address, 0, sizeof(server_address));
	strncpy(server_address, DEFAULT_SERVER_ADDRESS, sizeof(server_address) - 1);

#if defined(_WIN32) || defined(WIN32)
	WSADATA wsa_data;
	int startup_result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (startup_result != NO_ERROR) {
		fprintf(stderr, "Error at WSAStartup()\n");
		return EXIT_FAILURE;
	}
#endif

	// Parse CLI arguments
	int request_present = 0;
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-s") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "Missing value for -s option\n");
				fprintf(stderr, usage_format, argv[0]);
				goto cleanup;
			}
			strncpy(server_address, argv[++i], sizeof(server_address) - 1);
			server_address[sizeof(server_address) - 1] = '\0';
		} else if (strcmp(argv[i], "-p") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "Missing value for -p option\n");
				fprintf(stderr, usage_format, argv[0]);
				goto cleanup;
			}
			char *endptr = NULL;
			unsigned long port_value = strtoul(argv[++i], &endptr, 10);
			if (endptr == NULL || *endptr != '\0' || port_value == 0 || port_value > 65535) {
				fprintf(stderr, "Invalid port value: %s\n", argv[i]);
				fprintf(stderr, usage_format, argv[0]);
				goto cleanup;
			}
			server_port = (unsigned short) port_value;
		} else if (strcmp(argv[i], "-r") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "Missing value for -r option\n");
				fprintf(stderr, usage_format, argv[0]);
				goto cleanup;
			}
			if (parse_request(argv[++i], &request) != 0) {
				fprintf(stderr, "Invalid request format. Expected \"type city\".\n");
				fprintf(stderr, usage_format, argv[0]);
				goto cleanup;
			}
			request_present = 1;
		} else {
			fprintf(stderr, "Unknown argument: %s\n", argv[i]);
			fprintf(stderr, usage_format, argv[0]);
			goto cleanup;
		}
	}

	if (!request_present) {
		fprintf(stderr, "Missing required -r option\n");
		fprintf(stderr, usage_format, argv[0]);
		goto cleanup;
	}

	client_socket = connect_to_server(server_address, server_port);
	if (client_socket < 0) {
		fprintf(stderr, "Unable to connect to %s:%u\n", server_address, server_port);
		goto cleanup;
	}

	if (send_weather_request(client_socket, &request) != 0) {
		fprintf(stderr, "Failed to send weather request\n");
		goto cleanup;
	}

	if (receive_weather_response(client_socket, &response) != 0) {
		fprintf(stderr, "Failed to receive weather response\n");
		goto cleanup;
	}

	struct sockaddr_in peer_addr;
	memset(&peer_addr, 0, sizeof(peer_addr));
	socklen_t peer_len = sizeof(peer_addr);
	if (getpeername(client_socket, (struct sockaddr*) &peer_addr, &peer_len) == 0) {
		const char *resolved_ip = inet_ntoa(peer_addr.sin_addr);
		if (resolved_ip != NULL) {
			strncpy(server_ip, resolved_ip, sizeof(server_ip) - 1);
		}
	}
	if (server_ip[0] == '\0') {
		strncpy(server_ip, server_address, sizeof(server_ip) - 1);
	}
	server_ip[sizeof(server_ip) - 1] = '\0';

	if (format_response_message(&response, &request, server_ip, response_message, sizeof(response_message)) != 0) {
		fprintf(stderr, "Unable to format server response\n");
		goto cleanup;
	}

	printf("%s\n", response_message);
	exit_code = EXIT_SUCCESS;

cleanup:
	if (client_socket >= 0) {
		closesocket(client_socket);
	}
	clearwinsock();
	return exit_code;
} // main end
