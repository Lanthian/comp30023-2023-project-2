#include "rpc.h"
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

struct rpc_server {
    /* Add variable(s) for server state */
};

rpc_server *rpc_init_server(int port) {

    return NULL;
}

int rpc_register(rpc_server *srv, char *name, rpc_handler handler) {
    // Check f or null values
    if (srv == NULL || name == NULL || handler == NULL) return -1;
    // Check name length
    if (strlen(name) <= 0 || strlen(name) > 1000) return -1;


    return -1;
}

void rpc_serve_all(rpc_server *srv) {

}

struct rpc_client {
    /* Add variable(s) for client state */
    int sockfd;
    struct addrinfo hints;
};

struct rpc_handle {
    /* Add variable(s) for handle */
};

rpc_client *rpc_init_client(char *addr, int port) {
    rpc_client *client = malloc(sizeof(rpc_client));
    if (client == NULL) return NULL;            // Failed to allocate memory


    int n, s;
    struct addrinfo *servinfo, *rp;

    // Create address
    memset(&client->hints, 0, sizeof client->hints)
    client->hints.ai_family = AF_INET6;
    client->hints.ai_socktype = SOCK_STREAM;

    // Get addrinfo of server
    s = getaddrinfo(addr, port, &hints, &servinfo)
    if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

    // Connect to valid IPv6 result
    for (rp = servinfo; rp != NULL; p = p->ai_next) {
        if (rp->ai_family == AF_INET6
            && (sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) < 0) {

            }

    }

    client->sockfd

    return NULL;
}

rpc_handle *rpc_find(rpc_client *cl, char *name) {
    return NULL;
}

rpc_data *rpc_call(rpc_client *cl, rpc_handle *h, rpc_data *payload) {
    return NULL;
}

void rpc_close_client(rpc_client *cl) {
    if (c1 == NULL) return;
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
