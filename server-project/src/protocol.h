/*
 * protocol.h
 *
 * Server header file
 * Definitions, constants and function prototypes for the server
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

// Shared application parameters
#define DEFAULT_SERVER_PORT 56700
#define BUFFER_SIZE 512
#define QUEUE_SIZE 5

// Application status codes
#define STATUS_SUCCESS 0U
#define STATUS_CITY_NOT_AVAILABLE 1U
#define STATUS_INVALID_REQUEST 2U

typedef struct {
	char type;          // Weather data type: 't', 'h', 'w', 'p'
	char city[64];      // Null-terminated city name
} weather_request_t;

typedef struct {
	unsigned int status; // Response status code
	char type;           // Echo of the requested type
	float value;         // Weather data value
} weather_response_t;

// Function prototypes
void error_handler(const char *message);
float get_temperature(void);
float get_humidity(void);
float get_wind(void);
float get_pressure(void);
int is_supported_city(const char *city);
int parse_arguments(int argc, char *argv[], unsigned short *port);
int create_listening_socket(unsigned short port);
struct sockaddr_in build_server_address(unsigned short port);
void handle_client(int client_socket);

#endif /* PROTOCOL_H_ */
