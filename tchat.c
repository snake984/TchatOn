#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define IP 127.0.0.1

void *client (void *arg);

int main (int argc, char *argv []) {

	int sock=-1;

	if ((sock = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
		perror ("Couldn't initialize the socket\n");
		exit (-1);
	}

	struct sockaddr_in addr = {0};
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(0);
	

	if (bind (sock, (struct sockaddr*) &addr, sizeof addr) == -1) {
		perror ("Couldn't bind the socket\n");
		exit (-1);
	}


	if(listen(sock, 5) == -1)
	{
	    perror("Couldn't initialize the max connection\n");
	    exit(-1);
	}


	int c_sock;
	struct sockaddr_in c_addr = {0};
	socklen_t c_addr_size = sizeof c_addr;
	
	if ((c_sock = accept(sock, (struct sockaddr *) &c_addr, &c_addr_size)) == -1) {
		perror ("Couldn't accept the client\n");
		exit (-1);
	}
	else {
		pthread_t *t = malloc (sizeof (pthread_t));
		if (pthread_create(t, NULL, &client, (void *) &c_sock) != 0) {
			perror ("Couldn't initialize thread\n");
			exit (-1);
		}
	}

	close (sock);

	return 0;
}

void *client (void *arg) {

	int sock = *((int*)arg);
	if (send(sock, "Hello", 10, 0) == -1) {
		perror ("Couldn't send the message\n");
		exit (-1);
	}
	close (sock);

	return NULL;
}
