#include "rpc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// temp ?
#include <netdb.h>
#include <string.h>
#include <unistd.h>

// Constants for reading in arguments
#define FLAG_IP 'i'
#define FLAG_PORT 'p'
#define NO_IP NULL
#define NO_PORT -1
#define PARAM_NEEDED 2


int main(int argc, char *argv[]) {
    int exit_code = 0;

    // -- Read in arguments to run client --
    char *ip_address = NO_IP;
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
                case FLAG_IP:
                    ip_address = argv[++i];
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
    if (ip_address != NO_IP) param_read++;

    // Check enough arguments have been read in to instantiate client.
    if (param_read < PARAM_NEEDED) {       // todo - declare constant here 
        printf("Not enough arguments. (%d/%d)\n\tFormat: ./rpc-client -i server_ip_address -p port_num\n", param_read, PARAM_NEEDED);
        exit(EXIT_SUCCESS);
    } 
    // Temporary declaration here, just for cohesion of code            // todo
    // printf("IP: %s, Port: %d\n", ip_address, port_num);      // temprint

    // ======================== Actual client code from this point forth

    rpc_client *client = rpc_init_client(ip_address, port_num);       // todo take these values from command line
    if (client == NULL) {
        printf("rpc_init_client return fail.\n");
        exit(EXIT_FAILURE);
    }

    // printf("2\n");           // temprint
    rpc_handle *handle_add2 = rpc_find(client, "add2");
    if (handle_add2 == NULL) {
        fprintf(stderr, "ERROR: Function add2 does not exist\n");
        exit_code = 1;
        goto cleanup;
    }
    // todo - temp
    handle_add2 = rpc_find(client, "add2");
    if (handle_add2 == NULL) {
        fprintf(stderr, "ERROR: Function add2 does not exist\n");
        exit_code = 1;
        goto cleanup;
    }

    // printf("3 (using handle_add2 = %p)\n", handle_add2);         // temprint
    // return -1;

    for (int i = 0; i < 4; i++) {
        /* Prepare request */
        char left_operand = i;             // i
        char right_operand = 100;           // 100
        rpc_data request_data = {
            .data1 = left_operand, .data2_len = 1, .data2 = &right_operand};

        /* Call and receive response */
        rpc_data *response_data = rpc_call(client, handle_add2, &request_data);
        if (response_data == NULL) {
            fprintf(stderr, "Function call of add2 failed\n");
            exit_code = 1;
            goto cleanup;
        }

        /* Interpret response */
        assert(response_data->data2_len == 0);
        assert(response_data->data2 == NULL);
        printf("Result of adding %d and %d: %d\n", left_operand, right_operand,
               response_data->data1);
        rpc_data_free(response_data);
    }

cleanup:
    if (handle_add2 != NULL) {
        free(handle_add2);
    }

    rpc_close_client(client);
    client = NULL;

    return exit_code;
}
