CC=gcc
CFLAGS=-Wall -g -pthread

all: tchat r_client w_client

tchat:
	$(CC) $(CFLAGS) tchat.c -o tchat.o 

r_client:
	$(CC) $(CFLAGS) r_client.c -o r_client.o

w_client:
	$(CC) $(CFLAGS) w_client.c -o w_client.o

clean:
	rm *.o
