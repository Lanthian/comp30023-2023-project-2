#include "rpc.h"

#define _POSIX_C_SOURCE 200112L

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


struct rpc_server {
    /* Variable(s) for server state */
    int newsockfd;
    int sockfd;
    struct addrinfo hints;

    char* function_name;
    rpc_handler function;
};

rpc_server *rpc_init_server(int port) {
    printf("a\n");
    char port_str[6];
    sprintf(port_str, "%d", port);

    rpc_server *server = malloc(sizeof(rpc_server));
    if (server == NULL) return NULL;            // Failed to allocate memory

    // Given memory allocation is successful begin initiation
    int re, s;
    struct addrinfo *res;

    // Create address
    memset(&server->hints, 0, sizeof server->hints);
    server->hints.ai_family = AF_INET6;         // IPv6
	server->hints.ai_socktype = SOCK_STREAM;    // Connection-mode byte streams
	server->hints.ai_flags = AI_PASSIVE;        // for bind, listen, accept
    
    // Get addrinfo
    s = getaddrinfo(NULL, port_str, &server->hints, &res);
    if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        free(server);
        freeaddrinfo(res);
		exit(EXIT_FAILURE);         // todo -change to return NULL
	}

    // Create socket
	server->sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (server->sockfd < 0) {
		perror("socket");
        free(server);
        freeaddrinfo(res);
		exit(EXIT_FAILURE);         // todo -change to return NULL
	}

    // Reuse port if possible
    re = 1;
	if (setsockopt(server->sockfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(int)) < 0) {
		perror("setsockopt");
        close(server->sockfd);
        free(server);
        freeaddrinfo(res);
		exit(EXIT_FAILURE);         // todo -change to return NULL
	}
	// Bind address to the socket
	if (bind(server->sockfd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("bind");
        close(server->sockfd);
        free(server);
        freeaddrinfo(res);
		exit(EXIT_FAILURE);         // todo -change to return NULL
	}
	freeaddrinfo(res);


    // -- Successfully initiated server socket, set to listen --
    if (listen(server->sockfd, 10) < 0) {    // todo - find out what this number means
        perror("listen");
        close(server->sockfd);
        free(server);
        exit(EXIT_FAILURE);
    }

    // Given listen is successful, return server (not yet blocked)
    printf("b\n");

	return server;
}

int rpc_register(rpc_server *srv, char *name, rpc_handler handler) {
    // Check f or null values
    if (srv == NULL || name == NULL || handler == NULL) return -1;
    // Check name length
    if (strlen(name) <= 0 || strlen(name) > 1000) return -1;

    
    srv->function_name = name;
    srv->function = handler;

    return REGISTER_SUCCESS;
}

void rpc_serve_all(rpc_server *srv) {
    printf("---Serving!---\n");
}

struct rpc_client {
    /* Variable(s) for client state */
    int sockfd;
    struct addrinfo hints;
};

struct rpc_handle {
    /* Add variable(s) for handle */
};

rpc_client *rpc_init_client(char *addr, int port) {
    printf("i\n");
    rpc_client *client = malloc(sizeof(rpc_client));
    if (client == NULL) return NULL;            // Failed to allocate memory

    printf("iii\n");
    // Given memory allocation is successful begin initiation
    int s;
    struct addrinfo *servinfo, *rp;

    // Create address
    memset(&client->hints, 0, sizeof client->hints);
    client->hints.ai_family = AF_INET6;
    client->hints.ai_socktype = SOCK_STREAM;

    printf("iv\n");
    // Port as char*
    char port_str[6];
    sprintf(port_str, "%d", port);

    // Get addrinfo of server
    s = getaddrinfo(addr, port_str, &client->hints, &servinfo);
    if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        free(client);
		exit(EXIT_FAILURE);
	}
    printf("v\n");

    // Connect to valid result
    for (rp = servinfo; rp != NULL; rp = rp->ai_next) {
        // Only accept IPv6
        if (rp->ai_family == AF_INET6) {

            client->sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (client->sockfd < 0) continue;                                           // Invalid socket
            if (connect(client->sockfd, rp->ai_addr, rp->ai_addrlen) != -1) break;      // Success
            else close(client->sockfd);
        }
    }
    // If socket was not successfully connected, return a fail (NULL).
    if (rp == NULL) {
        free(client);
        freeaddrinfo(servinfo);
        return NULL;
    }
    freeaddrinfo(servinfo);

    // Otherwise return successfully linked client
    printf("ii\n");
    return client;
}

rpc_handle *rpc_find(rpc_client *cl, char *name) {
    return NULL;
}

rpc_data *rpc_call(rpc_client *cl, rpc_handle *h, rpc_data *payload) {
    return NULL;
}

void rpc_close_client(rpc_client *cl) {
    if (cl == NULL) return;
    // todo - check if socket already closed
    else {
        // Close and free client
        close(cl->sockfd);
        free(cl);
        cl = NULL;
    }
}

void rpc_data_free(rpc_data *data) {
    if (data == NULL) {
        return;
    }
    if (data->data2 != NULL) {
        free(data->data2);
    }
    free(data);
}
