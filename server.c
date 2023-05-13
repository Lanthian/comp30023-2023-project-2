#include "rpc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

rpc_data *add2_i8(rpc_data *);

int main(int argc, char *argv[]) {
    rpc_server *server;

    server = rpc_init_server(PORT_NUM);
    if (server == NULL) {
        fprintf(stderr, "Failed to init\n");
        exit(EXIT_FAILURE);
    }

    if (rpc_register(server, "add2", add2_i8) == -1) {
        fprintf(stderr, "Failed to register add2\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 1; i++) {
        rpc_print_handle(get_server_handle(server));
    }

    // todo - check if rpc_register done properly

    // temp - testing add2
    char left_operand = 13;             // i
    char right_operand = 101;           // 100
    rpc_data request_data = {
        .data1 = left_operand, .data2_len = 1, .data2 = &right_operand};
    
    // rpc_print_data(&request_data);
    // rpc_print_data(add2_i8(&request_data));
    test_func_handle(server, &request_data);


    printf("Serving all right now:\n");
    rpc_serve_all(server);
    printf("Somehow done serving all...?\n");

    return 0;
}

/* Adds 2 signed 8 bit numbers */
/* Uses data1 for left operand, data2 for right operand */
rpc_data *add2_i8(rpc_data *in) {
    printf("--FUNCTION ADD2_I8 CALLED--\n");
    /* Check data2 */
    if (in->data2 == NULL || in->data2_len != 1) {
        return NULL;
    }

    /* Parse request */
    char n1 = in->data1;
    char n2 = ((char *)in->data2)[0];

    /* Perform calculation */
    printf("add2: arguments %d and %d\n", n1, n2);
    int res = n1 + n2;

    /* Prepare response */
    rpc_data *out = malloc(sizeof(rpc_data));
    assert(out != NULL);
    out->data1 = res;
    out->data2_len = 0;
    out->data2 = NULL;
    return out;
}
