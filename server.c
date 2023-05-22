#include "rpc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// Constants for reading in arguments
#define FLAG_PORT 'p'
#define NO_PORT -1
#define PARAM_NEEDED 1

rpc_data *add2_i8(rpc_data *);


int main(int argc, char *argv[]) {
    // -- Read in arguments to run client --
    int port_num = NO_PORT;

    // Start from 1 to skip executable argument
    for (int i=1; i<argc; i++) {
        // Look for '-x' where x is some known flag.
        if ((argv[i][0] == '-') && (i+1 < argc)) {

            // Check for missing flag value
            if (argv[i+1][0] == '-') {
                printf("ERROR: Missing flag value.\n");
                return(EXIT_SUCCESS);
            }

            switch(argv[i][1]) {
                // Also increment i to skip next term
                case FLAG_PORT:
                    port_num = atoi(argv[++i]);
                    break;
                default:
                    printf("ERROR: Unknown flag entered.\n");
                    return(EXIT_SUCCESS);
            }
        }
    }

    // Check enough parameters have been read in 
    int param_read = 0;
    if (port_num != NO_PORT) param_read++;

    // Check enough arguments have been read in to instantiate client.
    if (param_read < PARAM_NEEDED) {       // todo - declare constant here 
        printf("Not enough arguments. (%d/%d)\n\tFormat: ./rpc-server -p port_num\n", param_read, PARAM_NEEDED);
        exit(EXIT_SUCCESS);
    } 

    // ======================== Actual server code from this point forth

    rpc_server *server = rpc_init_server(port_num);
    if (server == NULL) {
        fprintf(stderr, "Failed to init\n");
        exit(EXIT_FAILURE);
    }

    if (rpc_register(server, "add2", add2_i8) == -1) {
        fprintf(stderr, "Failed to register add2\n");
        exit(EXIT_FAILURE);
    }
    
    rpc_serve_all(server);

    return 0;
}

/* Adds 2 signed 8 bit numbers */
/* Uses data1 for left operand, data2 for right operand */
rpc_data *add2_i8(rpc_data *in) {
    // printf("--FUNCTION ADD2_I8 CALLED--\n"); // temprint
    /* Check data2 */
    if (in->data2 == NULL || in->data2_len != 1) {
        return NULL;
    }

    /* Parse request */
    char n1 = in->data1;
    char n2 = ((char *)in->data2)[0];

    /* Perform calculation */
    // printf("add2: arguments %d and %d\n", n1, n2);  // temprint
    int res = n1 + n2;

    /* Prepare response */
    rpc_data *out = malloc(sizeof(rpc_data));
    assert(out != NULL);
    out->data1 = res;
    out->data2_len = 0;
    out->data2 = NULL;
    return out;
}
