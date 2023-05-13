#include "rpc.h"

// #define _POSIX_C_SOURCE 200112L
#include <arpa/inet.h>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define MAX_FUNC_LENGTH 1000
#define MAX_INT_LENGTH 10

struct rpc_server {
    /* Variable(s) for server state */
    int newsockfd;
    int sockfd;
    struct addrinfo hints;

    struct sockaddr_in client_addr;
    socklen_t client_addr_size;

    // everything below here needs to be freed at some point
    rpc_handle* handle;
};

rpc_server *rpc_init_server(int port) {
    printf("a\n");
    char port_str[6];
    sprintf(port_str, "%d", port);

    rpc_server *server = malloc(sizeof(rpc_server));
    if (server == NULL) return NULL;            // Failed to allocate memory
    server->handle = NULL;          // todo --- will cause you a problem later hahha
    

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
        goto cleanup;
	}

    // Create socket
	server->sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (server->sockfd < 0) {
		perror("socket");
        goto cleanup;
	}

    // Reuse port if possible
    re = 1;
	if (setsockopt(server->sockfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(int)) < 0) {
		perror("setsockopt");
        goto cleanup;
	}
	// Bind address to the socket
	if (bind(server->sockfd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("bind");
        goto cleanup;
	}
	freeaddrinfo(res);


    // -- Successfully initiated server socket, set to listen --
    if (listen(server->sockfd, 10) < 0) {    // todo - find out what this number means
        perror("listen");
        goto cleanup;
    }

    // Given listen is successful, return server (not yet blocked)
    printf("b\n");

	return server;

cleanup:
    if (server->sockfd >= 0) close(server->sockfd);
    if (server != NULL) free(server);
    if (res != NULL) free(res);

    return NULL;
}

struct rpc_handle {
    /* Add variable(s) for handle */
    int handle_id;
    char function_name[MAX_FUNC_LENGTH];
    rpc_handler function;
};

int rpc_register(rpc_server *srv, char *name, rpc_handler handler) {
    // Check f or null values
    if (srv == NULL || name == NULL || handler == NULL) return -1;
    // Check name length
    if (strlen(name) <= 0 || strlen(name) > MAX_FUNC_LENGTH) return -1;

    
    srv->handle = malloc(sizeof(rpc_handle));
    if (srv->handle == NULL) {
        // no room in memory for malloc
        return -1;
    }

    srv->handle->handle_id = 12;                // todo / temp
    strcpy(srv->handle->function_name, name);
    srv->handle->function = handler;                // todo handler

    return REGISTER_SUCCESS;
}

void rpc_serve_all(rpc_server *srv) {
    printf("---Serving!---\n");
    print_server_handle(srv);
    printf("Didn't expect that to work...\n");


    struct sockaddr_in client_addr;
    socklen_t client_addr_size;

    srv->client_addr_size = sizeof srv->client_addr;
	srv->newsockfd = accept(srv->sockfd, (struct sockaddr*)&client_addr, 
                            &(srv->client_addr_size));
    // srv->newsockfd = accept(srv->sockfd, (struct sockaddr*) &(srv->client_addr), 
    //                         &(srv->client_addr_size));
    

    printf("K");
    print_server_handle(srv);


	if (srv->newsockfd < 0) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

    printf("CONNECTION ESTABLISHED!!!\n");

    char ip[INET6_ADDRSTRLEN];
    int port;

        printf("V-- here");
    print_server_handle(srv);

    getpeername(srv->newsockfd, (struct sockaddr*)&client_addr, 
                            &(srv->client_addr_size));
    // getpeername(srv->newsockfd, (struct sockaddr*) &(srv->client_addr), 
    //                         &(srv->client_addr_size));

        printf("&*()");
    print_server_handle(srv);

    // inet_ntop(srv->client_addr.sin_family, &srv->client_addr.sin_addr, ip, INET6_ADDRSTRLEN);

        printf("ASD");
    print_server_handle(srv);

    // port = ntohs(srv->client_addr.sin_port);
    // printf("new connection from %s:%d on socket %d\n", ip, port, srv->newsockfd);

        printf("V");
    print_server_handle(srv);
    

    // // ========================================================================
    // // temp change to test read-write
    // char buffer[256];
    // // Read characters from the connection, then process
	// int n = read(srv->newsockfd, buffer, 255); // n is number of characters read
	// if (n < 0) {
	// 	perror("read");
	// 	exit(EXIT_FAILURE);
	// }
	// // Null-terminate string
	// buffer[n] = '\0';

	// // Write message back
	// printf("Here is the message: %s\n", buffer);
	// n = write(srv->newsockfd, "I got your message\n", 19);
	// if (n < 0) {
	// 	perror("write");
	// 	exit(EXIT_FAILURE);
	// }
    // // ........................................................................


    printf("Read in function call\n");
    // print_server_handle(srv);

    // Read in function call requests
    printf("xxx, %d\n", 42);
    char func_name[MAX_FUNC_LENGTH];
    int n = read(srv->newsockfd, func_name, MAX_FUNC_LENGTH-1);    // n is the number of characters read
    if (n < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    printf("y111\n");

        printf("CC");
    print_server_handle(srv);

    func_name[n] = '\0';
    printf("yyy, %d\n", n);
    // Null-terminate string
    printf("ssssssssssssssssssss\n");
    printf("%d\n", 2);
    printf("why %d > %d         ", 42, 42);

        printf("L");
    print_server_handle(srv);

    printf("compare name w/ existing function name\n");
    // Check if handle exists           // todo - properly please lym
    printf("Handle: %s, Searched: %s\n", srv->handle->function_name, func_name);
    if (strcmp(srv->handle->function_name, func_name) != 0) {                   // strcmp(srv->handle->function_name, func_name)
        // Function doesn't exist
        printf("Function %s not found.\n", func_name);
        printf("Try %s instead.\n", srv->handle->function_name);
        return;
    }
    printf("um..\n");

    // Write message back
    char id[MAX_INT_LENGTH];
    snprintf(id, MAX_INT_LENGTH, "%d", 42);

	printf("Function %s requested.\n", func_name);
	n = write(srv->newsockfd, id, MAX_INT_LENGTH);
	if (n < 0) {
		perror("write");
		exit(EXIT_FAILURE);
	}   

    printf("No errors.\n");
    // printf("%s", srv->handle->function_name);
        printf("H");
    print_server_handle(srv);

    printf("Try using func calls now\n");
    while (1) {
        char func_handle[3];        // can take 0-99 handles
        
        int n = read(srv->newsockfd, func_handle, 3-1);    // n is the number of characters read
        if (n < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        func_handle[n] = '\0';

        printf("function request for id [%s] received.\n", func_handle);
        // todo - thread here? Or earlier... Dunno.

        
        break;
    }
}

struct rpc_client {
    /* Variable(s) for client state */
    int sockfd;
    struct addrinfo hints;
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
    // Malloc space to return a handle
    rpc_handle *handle = malloc(sizeof(rpc_handle));
    if (handle == NULL) {
        perror("lack of memory");
        exit(EXIT_FAILURE);
    }

    printf("good sign!\n");
    handle->handle_id = -1;
    handle->function = NULL;
    // handle->function_name = NULL;

    // todo / temp
    char buffer[MAX_INT_LENGTH];           // allowing up to 10 decimal digit int IDs
    // Send name to server
    printf("ccc\n");
    int n = write(cl->sockfd, name, strlen(name));
    if (n < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    printf("ddd\n");
    // Read handle ID to construct handle from server
    n = read(cl->sockfd, buffer, MAX_INT_LENGTH-1);
    if (n < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    printf("eee\n");
    // Null-terminate string
    buffer[n] = '\0';
    

    // Transform string ID to int ID
    int x = atoi(buffer);
    if (x < 0) {
        // Function not found on server
        free(handle);
        return NULL;
    }

    // Otherwise search was successful. Return handle object.
    handle->handle_id = x;
    printf("prc_find is fine?\n");
    return handle;
}

#define temp 3

rpc_data *rpc_call(rpc_client *cl, rpc_handle *h, rpc_data *payload) {
    char handle_id[temp];
    snprintf(handle_id, temp, "%d", 23);

    int n = write(cl->sockfd, handle_id, temp);
    if (n < 0) {
		perror("write");
		exit(EXIT_FAILURE);
	}   
    printf("Wrote function call for function [%s] to server\n", handle_id);

    rpc_send_data(cl->sockfd, payload);

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


// temp 2023.05.11
int return_sockfd(rpc_client *client) {
    return client->sockfd;
}

void print_server_handle(rpc_server *server) {
    printf(">> %d\n", server->handle->handle_id);
    printf(">> %s\n", server->handle->function_name);
    printf(">> %p\n", server->handle->function);
}

void rpc_send_data(int socket, rpc_data *payload) {
    // write to a socket, translate data (via buffer and conversion) to string / byte format first.
    // different errors for each stage that can fail.
    if (payload == NULL) {
        printf("Error: No data.\n");
        exit(EXIT_FAILURE);
    }


    char data1_buffer[MAX_INT_LENGTH];
    snprintf(data1_buffer, MAX_INT_LENGTH, "%d", payload->data1);
    printf(".%s\n", data1_buffer);

    char data2_len[MAX_INT_LENGTH];
    snprintf(data2_len, MAX_INT_LENGTH, "%d", (int)payload->data2_len);
    printf("..%s\n", data2_len);

    char data2_buffer[payload->data2_len];
    strcpy(data2_buffer, (char*)payload->data2);
    // snprintf(data2_buffer, payload->data2_len, "%d", )
    printf("-%p\n", payload->data2);
    printf("--%s\n", (char*)payload->data2);
 
    printf("---");
    unsigned char *ptr = payload->data2;
    for (int i = 0; i < 8; i++) {
        printf("%02x ", (int)ptr[i]);
    }
    printf("\n");

    printf("...%s\n", data2_buffer);





    int n = write(socket, data1_buffer, MAX_INT_LENGTH);
	if (n < 0) {
		perror("read");
		exit(EXIT_FAILURE);
	}   
}

rpc_data *rpc_receive_data(int socket) {
    // read from a socket, translate data (via buffer and conversion, etc.) back to ideal data
    // malloc an rpc_data (can be moved to where this is called instead I guess)
    // remember to free this malloc later

    // rpc_data *data = malloc(sizeof(data));

    char data1_buffer[MAX_INT_LENGTH];
    char data_2_len[MAX_INT_LENGTH];
    char data2_buffer[MAX_INT_LENGTH];

    int n = read(socket, data1_buffer, MAX_INT_LENGTH);
	if (n < 0) {
		perror("read");
		exit(EXIT_FAILURE);
	}   

    printf("--> %s\n", data1_buffer);    
}

void rpc_print_data(rpc_data *data) {
    printf("__________________________________\n");
    char data1_buffer[MAX_INT_LENGTH];
    snprintf(data1_buffer, MAX_INT_LENGTH, "%d", data->data1);
    printf("data1: %s\n", data1_buffer);

    char data2_len[MAX_INT_LENGTH];
    snprintf(data2_len, MAX_INT_LENGTH, "%d", (int)data->data2_len);
    printf("data2_len: %s\n", data2_len);

    if (data->data2_len > 0) {
        unsigned char *data2_char = data->data2;
        printf("data2: %s\n", data2_char);
        // char data2_buffer[data->data2_len + 1];
        // strcpy(data2_buffer, (char*)data->data2);
        // data2_buffer[data->data2_len] = '\0';
        // // snprintf(data2_buffer, payload->data2_len, "%d", )
        // printf("-%p\n", data->data2);
        // printf("--%s\n", (char*)data->data2);
    
        // printf("---");
        // unsigned char ptr2[data->data2_len];
        // unsigned char *ptr = data->data2;
        // // printf("============= %s\n", ptr);
        // for (int i = 0; i < data->data2_len; i++) {
        //     printf("%02x ", (char)ptr[i]);
        //     data2_buffer[i] = (char)ptr[i];
        //     ptr2[i] = ((char *)data->data2)[i];
        // }
        // ptr2[data->data2_len] = '\0';
        // printf("\n");
        // void* unkow = ptr;
        // printf("/%p\n", unkow);
        // printf("//%s\n", (char*)unkow);
        // printf("\\\\%s\n", ptr2);



        // printf("...%s\n", data2_buffer);
    }
    // printf("==================================\n");
    printf("``````````````````````````````````\n");
}

void test_func_handle(rpc_server *srv, rpc_data *data) {
    rpc_print_data(data);

    printf("function handle: %p\n",srv->handle->function);
    printf("function name: %s\n", srv->handle->function_name);

    rpc_print_data((srv->handle->function)(data));
}