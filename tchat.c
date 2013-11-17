#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ifaddrs.h>
#include <netdb.h>


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
	socklen_t sock_len = sizeof addr;
	

	if (bind (sock, (struct sockaddr*) &addr, sock_len) == -1) {
		perror ("Couldn't bind the socket\n");
		exit (-1);
	}


	// GET PORT
	if (getsockname(sock, (struct sockaddr *)&addr, &sock_len) == -1) {
    		perror("getsockname");
	}
	else {
	    printf("\t port: %d\n", ntohs(addr.sin_port));
	}

	struct ifaddrs *ifa, *ifaddr;
	if (getifaddrs(&ifaddr) == -1) {
		perror ("getifaddrs");
		exit (-1);
	}

	// GET IP ADDR
	int s, family;
	char host [100];
	ifa = ifaddr;
         while (ifa != NULL) {
               if (ifa->ifa_addr == NULL)
                   continue;

               family = ifa->ifa_addr->sa_family;

               if (family == AF_INET) {
                   s = getnameinfo(ifa->ifa_addr,
                       sizeof(struct sockaddr_in),
                       host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                   if (s != 0) {
                       printf("getnameinfo() failed: %s\n", gai_strerror(s));
                       exit(-1);
                   }
                   printf("\t@IP: %s\n", host);
               }
		ifa = ifa->ifa_next;
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
