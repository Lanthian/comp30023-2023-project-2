#include "rpc.h"

// #define _POSIX_C_SOURCE 200112L
#include <arpa/inet.h>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <assert.h>


// Task 9
#define NONBLOCKING


#define RPC_TIMEOUT 60         // todo change back to 30


#define MAX_FUNC_NAME_LENGTH 1000
#define MAX_INT_LENGTH 11       // Includes space for a '\0' terminator
#define GARBAGE_FILL '\0'
#define PACKET_LIMIT 65535

#define MAX_HANDLES 10
#define MAX_HANDLE_ID_LENGTH 3          // should maybe be 2?

#define BACK_LOG 10

#define NO_HANDLE -2
#define NO_SOCKET -3
#define NO_RPC_DATA -4

#define FLAG_BUFFER_SIZE 3            //
// 0 reserved, as read flag fails return 0
#define SERVER_FLAG_EMPTY 0
#define SERVER_FLAG_FIND 1
#define SERVER_FLAG_CALL 2
#define SEND_FLAG_SUCCESS 3
#define SEND_FLAG_HANDLE_FAIL -5
#define SEND_FLAG_DATA2_TOO_BIG -6

// Function headers
int rpc_send_data(int socket, rpc_data *payload);
rpc_data *rpc_read_data(int socket);
int rpc_send_flag(int socket, int flag);
int rpc_read_flag(int socket);
int rpc_check_data(rpc_data *data);

void rpc_close_server(rpc_server *srv);

void rpc_print_data(rpc_data *data); // todo


// Global variable to handle in alarm state. Careful with usage.
rpc_server *global_srv = NULL;
void alarm_handler() {
    rpc_close_server(global_srv);
    // return;          // todo
    exit(EXIT_SUCCESS);     // todo - ideally would just end and not exit, letting serve_all return and numerous servers run. alas.
}

struct rpc_server {
    /* Variable(s) for server state */
    int newsockfd;
    int sockfd;
    struct addrinfo hints;

    struct sockaddr_in client_addr;
    socklen_t client_addr_size;

    // everything below here needs to be freed at some point
    rpc_handle* handles[MAX_HANDLES];
};

rpc_server *rpc_init_server(int port) {
    char port_str[6];
    sprintf(port_str, "%d", port);

    rpc_server *server = malloc(sizeof(rpc_server));
    if (server == NULL) return NULL;            // Failed to allocate memory
    for (int i = 0; i<MAX_HANDLES; i++) {
        server->handles[i] = NULL;
    }
    

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
    if (listen(server->sockfd, BACK_LOG) < 0) {    // todo - find out what this number means
        perror("listen");
        goto cleanup;
    }

    // Quickly set new socket address so it's not an unassigned value
    server->newsockfd = NO_SOCKET;

    // Set global server to server instance - note this means only 1 server can be run and closed properly at a time.
    global_srv = server;
    // Given listen is successful, return server (not yet blocked)
	return server;

cleanup:
    if (server->sockfd >= 0) close(server->sockfd);
    if (server != NULL) free(server);
    if (res != NULL) free(res);

    if (global_srv != NULL) global_srv = NULL;

    return NULL;
}

struct rpc_handle {
    /* Add variable(s) for handle */
    int handle_id;
    char function_name[MAX_FUNC_NAME_LENGTH];
    rpc_handler function;
};

int rpc_register(rpc_server *srv, char *name, rpc_handler handler) {
    // Check f or null values
    if (srv == NULL || name == NULL || handler == NULL) return -1;
    // Check name length
    if (strlen(name) <= 0 || strlen(name) > MAX_FUNC_NAME_LENGTH) return -1;

    // First look for existent handle to replace
    // for (int i = 0; i<MAX_HANDLES; i++)

    // Find available handle spot in server handles array
    int index = -1;
    for (int i = 0; i<MAX_HANDLES; i++) {
        // Search for duplicate name registered (overwrite)
        if (srv->handles[i] != NULL && strcmp(name, srv->handles[i]->function_name)==0) {
            free(srv->handles[i]);
            index = i;
            break;
        }

        // Otherwise locate first unused handle spot
        else if (srv->handles[i] == NULL && index == -1) {
            index = i;
        }
    }

    // Abort registration and parse back error if unsucessful
    if (index == -1) return -1;

    // Malloc and assign handle to server           // -- todo, change this to array
    srv->handles[index] = malloc(sizeof(rpc_handle));
    if (srv->handles[index] == NULL) {
        // no room in memory for malloc
        printf("No memory for malloc.\n");
        exit(EXIT_FAILURE);
    }

    // Allocate details to handle, returning handle index (ID)
    srv->handles[index]->handle_id = index;                 // Done for client side access
    strcpy(srv->handles[index]->function_name, name);		// Searched for with rpc_find
    srv->handles[index]->function = handler;                // Function pointer for server side

    return index;
}

/*
  Closes an rpc_server* instance, freeing all functions, closing sockets and
  finally freeing the server itself before exiting the program with
  EXIT_SUCCESS.
*/
void rpc_close_server(rpc_server *srv) {
    // Free function handles
    for (int index=0; index<MAX_HANDLES; index++) {
        if (srv->handles[index] != NULL) {
            free(srv->handles[index]);
            srv->handles[index] = NULL;
        }
    }
    
    // Close sockets
    close(srv->sockfd);
    if (srv->newsockfd != NO_SOCKET) close(srv->newsockfd);

    // Free server itself, then exit
    free(srv);
    srv = NULL;
}

void rpc_serve_all(rpc_server *srv) {
    // Define clean up function for server in case of accept timeout
    signal(SIGALRM, alarm_handler);
    signal(SIGINT, alarm_handler);

    while(1) {
        // Start timeout alarm
        alarm(RPC_TIMEOUT);

        // Accept a client
        srv->client_addr_size = sizeof srv->client_addr;
        srv->newsockfd = accept(srv->sockfd, (struct sockaddr*)&srv->hints, 
                                &(srv->client_addr_size));
        // todo - get client_addr into hints somehow.....

        if (srv->newsockfd < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }


        // Fork a thread to deal with new client
        pid_t c_pid = fork();
        
        // Check for fork error
        if (c_pid < 0) {
            perror("fork");
            // todo - free server.... (closing file descriptors maybe...)
            exit(EXIT_FAILURE);
        }

        // If Parent process - do nothing, looping again
        else if (c_pid > 0) {
            close(srv->newsockfd);          // todo?
            srv->newsockfd = NO_SOCKET;      // maybe close first?
            srv->client_addr_size = sizeof srv->client_addr;        // not needed perhaps if client_addr fixed
            continue;
        }
        // Otherwise, Child process - serve client


        // -- Printing connection details -- (commented out)
        // struct sockaddr_in client_addr;
        // char ip[INET6_ADDRSTRLEN];
        // int port;
        // getpeername(srv->newsockfd, (struct sockaddr*)&client_addr, 
        //                         &(srv->client_addr_size));
        // // getpeername(srv->newsockfd, (struct sockaddr*) &(srv->client_addr), 
        // //                         &(srv->client_addr_size));
        // inet_ntop(client_addr.sin_family, &client_addr.sin_addr, ip, INET6_ADDRSTRLEN);
        // // inet_ntop(srv->client_addr.sin_family, &(srv->client_addr.sin_addr), ip, INET6_ADDRSTRLEN);
        // port = ntohs(srv->client_addr.sin_port);
        // printf("New connection from %s:%d on socket %d\n", ip, port, srv->newsockfd);


        while(1) {
            // Read command flag
            int flag = rpc_read_flag(srv->newsockfd);
            int n;

            switch(flag){
                // rpc_find:
                case SERVER_FLAG_FIND:
                    // Read in function handle name (to be searched for)
                    char func_name[MAX_FUNC_NAME_LENGTH];
                    n = read(srv->newsockfd, func_name, MAX_FUNC_NAME_LENGTH-1);    // n is the number of characters read
                    if (n < 0) {
                        perror("read");
                        exit(EXIT_FAILURE);
                    }
                    func_name[n] = '\0';    // null terminate read data


                    // Check if handle exists
                    int handle_index = NO_HANDLE;
                    for (int i=0; i<MAX_HANDLES; i++) {

                        // Firstly check if a handle exists at index
                        if (srv->handles[i] != NULL) {
                            // Then check if it matches the name queried. 
                            if (strcmp(srv->handles[i]->function_name, func_name) == 0) {
                                // Match found
                                handle_index = i;
                                break;
                            }
                        }
                    }
                    // If handle_index == NO_HANDLE, function of searched name doesn't exist. Write back -1.


                    // Write message back
                    char id[MAX_HANDLE_ID_LENGTH];
                    snprintf(id, MAX_HANDLE_ID_LENGTH, "%d", handle_index);

                    n = write(srv->newsockfd, id, MAX_HANDLE_ID_LENGTH-1);            // todo change max int length to 3??? or 4????
                    if (n < 0) {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }   
                    break;


                // rpc_call:
                case SERVER_FLAG_CALL:
                    // Read in function handle_id (index in handles array)
                    char func_handle_id[MAX_HANDLE_ID_LENGTH];        // can take 0-99 handles
                    n = read(srv->newsockfd, func_handle_id, MAX_HANDLE_ID_LENGTH-1);    // n is the number of characters read      // todo fix this 3
                    if (n < 0) {
                        perror("read");
                        exit(EXIT_FAILURE);
                    }
                    func_handle_id[n] = '\0';   // null terminate read data

                    int handle_id = atoi(func_handle_id);
                    assert(handle_id != NO_HANDLE);     // technically not needed but doesn't hurt.

                    rpc_data *data = rpc_read_data(srv->newsockfd);
                    // Abort if rpc_call failed to send data for some reason.
                    if (data==NULL) break;

                    rpc_data* new_data = (srv->handles[handle_id]->function)(data);
                    rpc_send_data(srv->newsockfd, new_data);

                    // Free data, as malloced in rpc_read_data and handle
                    rpc_data_free(data);
                    data = NULL;
                    rpc_data_free(new_data);
                    data = NULL;

                    break;


                // No flag sent
                case(SERVER_FLAG_EMPTY):
                    // Free threaded memory malloc-d
                    rpc_close_server(srv);
                    exit(EXIT_SUCCESS);


                // Undeclared flag sent somehow
                default:
                    perror("unknown server_flag");
                    exit(EXIT_FAILURE);
            }
        }
    }
    // Code should never reach here
    assert(0);
}

struct rpc_client {
    /* Variable(s) for client state */
    int sockfd;
    struct addrinfo hints;
};

rpc_client *rpc_init_client(char *addr, int port) {
    rpc_client *client = malloc(sizeof(rpc_client));
    if (client == NULL) return NULL;            // Failed to allocate memory

    // Given memory allocation is successful begin initiation
    int s;
    struct addrinfo *servinfo, *rp;

    // Create address
    memset(&client->hints, 0, sizeof client->hints);
    client->hints.ai_family = AF_INET6;
    client->hints.ai_socktype = SOCK_STREAM;

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
    return client;
}

/* 
  Shorthand function to send (int) flags to a socket adress. Returns the number
  of bytes sent (should be < FLAG_BUFFER_SIZE, > 0)
*/
int rpc_send_flag(int socket, int flag) {
    // Convert data1 (int) and data2_len (size_t) to char*
    char flag_buffer[FLAG_BUFFER_SIZE];
    snprintf(flag_buffer, FLAG_BUFFER_SIZE, "%d", flag);

    // Fill garbage of write before sending
    for (int i=strlen(flag_buffer); i<FLAG_BUFFER_SIZE; i++) {
        flag_buffer[i] = GARBAGE_FILL;
    }

    int n = write(socket, flag_buffer, FLAG_BUFFER_SIZE-1);           // todo =FLAG_BUFFER_SIZE-1  ?
    if (n < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    return n;
} 

/* 
  Shorthand function to read (char*) flags from a socket adress. Returns the 
  flag as an int.
*/
int rpc_read_flag(int socket) {
    // Initiate buffers to read in rpc_data fields
    char flag_buffer[FLAG_BUFFER_SIZE];

    // Read in flag
    int n = read(socket, flag_buffer, FLAG_BUFFER_SIZE-1);
	if (n < 0) {
		// perror("read");       
        return SERVER_FLAG_EMPTY;
		// exit(EXIT_FAILURE);
	}   
    flag_buffer[n] = '\0';

    // Convert and return flag
    return atoi(flag_buffer);
}

rpc_handle *rpc_find(rpc_client *cl, char *name) {
    // Malloc space to return a handle
    rpc_handle *handle = malloc(sizeof(rpc_handle));
    if (handle == NULL) {
        perror("lack of memory");
        goto cleanup;
    }

    // Fill temporary values for handle
    handle->handle_id = NO_HANDLE;
    handle->function = NULL;				// Client doesn't need access to server side function pointer
    // handle->function_name = name;        // -- todo fix issue here


    // Send flag to server to let it know rpc_find has been called
    rpc_send_flag(cl->sockfd, SERVER_FLAG_FIND);

    // Send function name to server
    int n = write(cl->sockfd, name, strlen(name));
    if (n < 0) {
        perror("socket");
        goto cleanup;
    }

    // Read handle ID to construct handle from server
    char buffer[MAX_INT_LENGTH];
    n = read(cl->sockfd, buffer, MAX_INT_LENGTH-1);
    if (n < 0) {
        perror("read");
        goto cleanup;
    }
    // Null-terminate string
    buffer[n] = '\0';
    
    // Transform string ID to int ID
    int x = atoi(buffer);
    if (x == NO_HANDLE) {
        // Function not found on server
        free(handle);
        return NULL;
    }

    // Otherwise search was successful. Return handle object.
    handle->handle_id = x;
    return handle;

cleanup:
    if (handle!=NULL) free(handle);
    exit(EXIT_FAILURE);
}


rpc_data *rpc_call(rpc_client *cl, rpc_handle *h, rpc_data *payload) {
    // Make sure data and handle exist
    if (rpc_check_data(payload)<0) return NULL;
    if (h == NULL) return NULL;
    

    // Check if protocol can send data2
    if (payload->data2_len > PACKET_LIMIT) {
        perror("Overlength error");
        return NULL;
    }

    // Convert handle_id to char* for sending
    char handle_id[MAX_HANDLE_ID_LENGTH];
    snprintf(handle_id, MAX_HANDLE_ID_LENGTH, "%d", h->handle_id);

    // Send flag to server to let it know rpc_call has been called
    rpc_send_flag(cl->sockfd, SERVER_FLAG_CALL);

    // Write handle id to server before sending data
    int n = write(cl->sockfd, handle_id, MAX_HANDLE_ID_LENGTH-1);
    if (n < 0) {
		perror("write");
		exit(EXIT_FAILURE);
	}   

    // Send data, double checking that send was successful
    n = rpc_send_data(cl->sockfd, payload);
    if (n<0) return NULL;

    return rpc_read_data(cl->sockfd);;
}

void rpc_close_client(rpc_client *cl) {
    if (cl == NULL) return;
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


/*
  Writes data fields of rpc_data* `payload` to socket file descriptor `socket`,
  converting fields to string format first to deal with endianness. First sends 
  a single digit flag to let symmetric rpc_read_data know if followed through
  or aborted.
  Returns an int indicator send success, or if fail, the specific error flag.
*/
int rpc_send_data(int socket, rpc_data *payload) {
    // Check data is not NULL 
    if (rpc_check_data(payload)) {
        /* Data is bad, either due to code mismanagement, or a failed handle 
        being applied to it. Send a flag to let reading socket know to abort. */
        rpc_send_flag(socket, SEND_FLAG_HANDLE_FAIL);
        return SEND_FLAG_HANDLE_FAIL;
    } 

    // Check if protocol can send data2
    else if (payload->data2_len > PACKET_LIMIT) {
        printf("Error: Data2 too large to send as a packet.\n");
        rpc_send_flag(socket, SEND_FLAG_DATA2_TOO_BIG);
        return SEND_FLAG_DATA2_TOO_BIG;
    }

    // Otherwise, send an affirmation flag to give reading socket the go ahead
    rpc_send_flag(socket, SEND_FLAG_SUCCESS);


    // Initiate buffers to store (and later write) converted rpc_data fields
    char int_buffer[MAX_INT_LENGTH];


    // Convert data1 (int) to char*
    snprintf(int_buffer, MAX_INT_LENGTH, "%d", payload->data1);
    // Fill remaining buffer garbage with temp garbage value
    /* Buffer has to be written as MAX_INT_LENGTH -1 instead of strlen(buffer)
    as inorder to ensure that the max possible int is read on the read side
    each time, MAX_INT_LENGTH-1 must be read.*/
    for (int i=strlen(int_buffer); i<MAX_INT_LENGTH; i++) {
        int_buffer[i] = GARBAGE_FILL;
    }
    // Write data1
    int n = write(socket, int_buffer, MAX_INT_LENGTH-1);
    if (n < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }


    // Convert dat2_len (size_t) to char*
    snprintf(int_buffer, MAX_INT_LENGTH, "%d", (int)payload->data2_len);
    // Fill remaining buffer garbage with temp garbage value
    for (int i=strlen(int_buffer); i<MAX_INT_LENGTH; i++) {
        int_buffer[i] = GARBAGE_FILL;
    }
    // Write data2_len
    n = write(socket, int_buffer, MAX_INT_LENGTH-1);        //MAX_INT_LENGTH-1
    if (n < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }


    // Convert data2, if it exists
    if (payload->data2_len > 0) {
        unsigned char *data2_buffer = payload->data2;

        // And then write data2
        n = write(socket, data2_buffer, payload->data2_len);
        if (n < 0) {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }

    // Packet successfully sent
    return 0;
}


/*
  Reads data fields of rpc_data* from a socket file descriptor `socket`,
  converting fields from string format and mallocing a returned rpc_data 
  pointer. Returns NULL on fail. rpc_data* returned otherwise needs to be freed
  later (malloc-d here).
*/
rpc_data *rpc_read_data(int socket) {
    // Read flag from socket - abort if send fail
    int flag = rpc_read_flag(socket);
    if (flag<0) {
        return NULL;
    }

    // Allocate returned data, checking if space to malloc
    rpc_data *return_data = malloc(sizeof(rpc_data));
    if (return_data == NULL) {
        perror("malloc");
        goto cleanup;
    }

    // Initiate buffers to read in rpc_data fields
    char int_buffer[MAX_INT_LENGTH];

    // Read in data1
    int n = read(socket, int_buffer, MAX_INT_LENGTH-1);
	if (n < 0) {
		perror("read");
		goto cleanup;
	}   
    int_buffer[n] = '\0';
    // Convert char* data1 to int and store
    return_data->data1 = atoi(int_buffer);


    // Read in data2_len
    n = read(socket, int_buffer, MAX_INT_LENGTH-1);
	if (n < 0) {
		perror("read");
		goto cleanup;
	}   
    int_buffer[n] = '\0';
    // Convert char* data1 to size_t and store
    return_data->data2_len = (size_t) atoi(int_buffer);

    // Check if data2 needs to be read
    if (return_data->data2_len > 0) {
        // Malloc space for void* pointer
        return_data->data2 = (void*)malloc(return_data->data2_len + 1);

        // Read in char* form of data2
        char data2_buffer[return_data->data2_len + 1];
        n = read(socket, data2_buffer, return_data->data2_len);
        if (n < 0) {
            perror("read");
            goto cleanup;
        }   
        data2_buffer[n] = '\0';

        // Copy/store read in data in/to return_data (of type void*)
        memcpy(return_data->data2, data2_buffer, return_data->data2_len);
    }
    else {
        // Otherwise, no data2 field, set NULL
        return_data->data2 = NULL;
    }

    // Return read data
    return return_data;

cleanup:
    if (return_data != NULL) free(return_data);
    exit(EXIT_FAILURE);
}

int rpc_check_data(rpc_data *data) {
    if (data == NULL) return -1;
    if (data->data2_len < 0) return -1;
    else if (data->data2_len == 0 && data->data2 != NULL) return -1;
    else if (data->data2_len != 0 && data->data2 == NULL) return -1;
    // printf("..%ld %ld\n", data->data2_len, strlen(data->data2));     // not checkable right now...
    return 0;
}

// rpc_handle *get_server_handle(rpc_server *srv) {
//     return (srv->handle);
// }

void rpc_print_handle(rpc_handle *handle) {
    printf("handle_id: %d\n", handle->handle_id);
    printf("function_name: %s\n", handle->function_name);
    printf("function handle: %p\n", handle->function);
}

void rpc_print_data(rpc_data *data) {
    printf("__________________________________\n");
    printf("data1: %d\n", data->data1);
    printf("data2_len: %d\n", (int)data->data2_len);
    if (data->data2_len > 0) {
        printf("data2: ");
        for (int i=0; i<(int)data->data2_len; i++) {
            printf("%c", ((unsigned char*)data->data2)[i]);
        }
        printf("\n");
    }
    //printf("data2: %s\n", (unsigned char*)data->data2);
    printf("``````````````````````````````````\n");
}

// // Print rpc_data before and after an operation of srv->handle.
// void test_func_handle(rpc_server *srv, rpc_data *data) {
//     // Print initial data
//     rpc_print_data(data);
//     // Print operation details
//     rpc_handle *func = get_server_handle(srv);
//     rpc_print_handle(func);
//     // Print operated on data
//     rpc_print_data((func->function)(data));
// }