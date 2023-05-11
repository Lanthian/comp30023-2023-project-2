#include "rpc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// temp ?
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int exit_code = 0;

    printf("1\n");
    if (argc < 2) {
        printf("Not enough arguments\n");
    } 
    printf("IP: %s\n", argv[1]);


    rpc_client *client = rpc_init_client(argv[1], PORT_NUM);       // todo take these values from command line
    if (client == NULL) {
        printf("rpc_init_client return fail.\n");
        exit(EXIT_FAILURE);
    }


    printf("1.5\n");
    // ========================================================================
    // temp change to test read-write
    char buffer[256];
    // Read message from stdin
	printf("Please enter the message: ");
	if (fgets(buffer, 255, stdin) == NULL) {
		exit(EXIT_SUCCESS);
	}
	// Remove \n that was read by fgets
	buffer[strlen(buffer) - 1] = 0;

    // Send message to server
	int n = write(return_sockfd(client), buffer, strlen(buffer));
	if (n < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}    
    // Read message from server
	n = read(return_sockfd(client), buffer, 255);
	if (n < 0) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	// Null-terminate string
	buffer[n] = '\0';
	printf("%s\n", buffer);
    // ........................................................................


    printf("2\n");
    rpc_handle *handle_add2 = rpc_find(client, "add2");
    if (handle_add2 == NULL) {
        fprintf(stderr, "ERROR: Function add2 does not exist\n");
        exit_code = 1;
        goto cleanup;
    }

    printf("3\n");
    for (int i = 0; i < 2; i++) {
        /* Prepare request */
        char left_operand = i;
        char right_operand = 100;
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
