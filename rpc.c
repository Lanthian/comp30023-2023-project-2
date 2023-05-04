#include "rpc.h"
#include <stdlib.h>
#include <string.h>

struct rpc_server {
    /* Add variable(s) for server state */
};

rpc_server *rpc_init_server(int port) {
    
    return NULL;
}

int rpc_register(rpc_server *srv, char *name, rpc_handler handler) {
    // Check for null values
    if (srv == NULL || name == NULL || handler == NULL) return -1;
    // Check name length
    if (strlen(name) <= 0 || strlen(name) > 1000) return -1;


    return -1;
}

void rpc_serve_all(rpc_server *srv) {

}

struct rpc_client {
    /* Add variable(s) for client state */

};

struct rpc_handle {
    /* Add variable(s) for handle */
};

rpc_client *rpc_init_client(char *addr, int port) {
    return NULL;
}

rpc_handle *rpc_find(rpc_client *cl, char *name) {
    return NULL;
}

rpc_data *rpc_call(rpc_client *cl, rpc_handle *h, rpc_data *payload) {
    return NULL;
}

void rpc_close_client(rpc_client *cl) {

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
