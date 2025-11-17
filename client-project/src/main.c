/*
 * main.c
 *
 * TCP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP client
 * portable across Windows, Linux and macOS.
 */

#if defined WIN32
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
#include "protocol.h"

#define NO_ERROR 0

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

	int pton_result = inet_pton(AF_INET, address, &sad.sin_addr);
	if (pton_result <= 0) {
		struct hostent *host = gethostbyname(address);
		if (host == NULL || host->h_addr_list == NULL || host->h_addr_list[0] == NULL) {
			closesocket(client_socket);
			return -1;
		}
		memcpy(&sad.sin_addr, host->h_addr_list[0], host->h_length);
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

int main(int argc, char *argv[]) {

	// TODO: Implement client logic

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return EXIT_SUCCESS;
	}
#endif

	// create client socket
	int c_socket;
	c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (c_socket < 0) {
		error_handler("socket creation failed.\n");
		closesocket(c_socket);
		clearwinsock();
		return EXIT_FAILURE;
	}

// set connection settings
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP del server
	sad.sin_port = htons(PROTO_PORT); // Server port

	// connection
	if (connect(c_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		error_handler("Failed to connect.\n");
		closesocket(c_socket);
		return EXIT_FAILURE;
	}

	// receive from server
	char buffer[BUFFER_SIZE];
	memset(buffer, '\0', BUFFER_SIZE);
	if ((recv(c_socket, buffer, BUFFER_SIZE - 1, 0)) <= 0) {
		error_handler("recv() failed or connection closed prematurely");
		closesocket(c_socket);
		return EXIT_FAILURE;
	}
	printf("%s\n", buffer); // Print the echo buffer


		MessageStruct msg;

	// 1. ALLOCAZIONE DELLA MEMORIA per i puntatori A e B
	msg.A = (char*)malloc(MAX_MSG_LEN);
	msg.B = (char*)malloc(MAX_MSG_LEN);

	if (msg.A == NULL || msg.B == NULL) {
		errorhandler("Errore nell'allocazione di memoria.\n");
		// Cleanup se l'allocazione fallisce
		if (msg.A) free(msg.A);
		if (msg.B) free(msg.B);
		closesocket(c_socket);
		clearwinsock();
		return EXIT_FAILURE;
	}


	// TODO: Configure server address
	// struct sockaddr_in server_addr;
	// ...

	// TODO: Connect to server
	// connect(...);

	// TODO: Implement communication logic
	// send(...);
	// recv(...);

	// TODO: Close socket
	// closesocket(my_socket);

	printf("Client terminated.\n");

	clearwinsock();
	return 0;
} // main end
