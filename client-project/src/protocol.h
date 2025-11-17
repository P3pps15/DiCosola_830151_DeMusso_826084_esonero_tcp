/*
 * protocol.h
 *
 * Client header file
 * Definitions, constants and function prototypes for the client
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stddef.h>

// Shared application parameters
#define DEFAULT_SERVER_PORT 56700
#define DEFAULT_SERVER_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 512
#define MAX_CITY_LEN 64
#define RESPONSE_MESSAGE_LEN 256

// Response status codes
#define STATUS_SUCCESS 0
#define STATUS_CITY_NOT_AVAILABLE 1
#define STATUS_INVALID_REQUEST 2

typedef struct {
    char type;                // 't', 'h', 'w', 'p'
    char city[MAX_CITY_LEN];  // Null-terminated city name
} weather_request_t;

typedef struct {
    unsigned int status;
    char type;
    float value;
} weather_response_t;

// Weather data generator prototypes (implemented on server side)
float get_temperature(void);
float get_humidity(void);
float get_wind(void);
float get_pressure(void);

// Client helper prototypes
int parse_request(const char *request_arg, weather_request_t *out_request);
int connect_to_server(const char *server_address, unsigned short port);
int send_weather_request(int socket_fd, const weather_request_t *request);
int receive_weather_response(int socket_fd, weather_response_t *response);
int format_response_message(const weather_response_t *response,
                            const weather_request_t *request,
                            const char *server_ip,
                            char *out_buffer,
                            size_t out_size);

#endif /* PROTOCOL_H_ */
