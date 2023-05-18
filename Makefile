CC=gcc
RPC_SYSTEM=rpc.o

.PHONY: format all

all: $(RPC_SYSTEM) server client

$(RPC_SYSTEM): rpc.c rpc.h
	$(CC) -c -o $@ $<

server.o: server.c rpc.h
	$(CC) -c -o $@ $<

client.o: client.c rpc.h
	$(CC) -c -o $@ $<

server: server.o $(RPC_SYSTEM)
	$(CC) -Wall -o server server.o $(RPC_SYSTEM)

client: client.o $(RPC_SYSTEM)
	$(CC) -Wall -o client client.o $(RPC_SYSTEM)

# RPC_SYSTEM_A=rpc.a
# $(RPC_SYSTEM_A): rpc.o
# 	ar rcs $(RPC_SYSTEM_A) $(RPC_SYSTEM)

format:
	clang-format -style=file -i *.c *.h


clean:
	rm *.o
	rm server client