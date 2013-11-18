#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>


void *client (void *arg);
void print_ip (char ip []);
int create_msg_queue ();
void read_msg (int msqid);
void write_msg (int msqid, char msg []);

int main (int argc, char *argv []) {

	int sock=-1;

	if ((sock = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
		perror ("socket");
		exit (-1);
	}

	char ip [50];
	print_ip (ip);

	struct sockaddr_in addr;
	inet_aton(ip, &addr.sin_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(0);
	socklen_t sock_len = sizeof addr;
	

	if (bind (sock, (struct sockaddr*) &addr, sock_len) == -1) {
		perror ("bind");
		exit (-1);
	}

	if(listen(sock, 5) == -1)
	{
	    perror("Couldn't initialize the max connection\n");
	    exit(-1);
	}



	// Print port
	if (getsockname(sock, (struct sockaddr *)&addr, &sock_len)
		== -1)
	{
    		perror("getsockname");
	}
	else
	{
	    printf("\tport: %d\n", ntohs(addr.sin_port));
	}

//	int test = create_msg_queue ();
//	write_msg (test, "Coucou\n");
//	read_msg(test);

	int c_sock;
	struct sockaddr_in c_addr = {0};
	socklen_t c_addr_size = sizeof c_addr;
	
	if ((c_sock = accept(sock, (struct sockaddr *) &c_addr,
			     &c_addr_size))
		 == -1)
	{
		perror ("accept\n");
		exit (-1);
	}
	else
	{
		pthread_t *t = malloc (sizeof (pthread_t));

		if (pthread_create(t, NULL, &client,
				   (void *) &c_sock) 
			!= 0) 
		{
			perror ("pthread_create");
			exit (-1);
		}
	}

	close (sock);

	return 0;
}

void *client (void *arg) {

	int sock = *((int*)arg);

	char msg [50];

	printf ("Client connected\n");
	if (send(sock, "Hello\n", 10, 0) == -1)
	{
		perror ("send");
		exit (-1);
	}

	if (recv(sock, msg, sizeof msg, 0))
	{
		perror ("recv");
		exit (-1);
	}

	if (strcmp (msg, "W"))
	{

	}
	else if (strcmp (msg, "R"))
	{

	}
	else
	{
		printf ("Couldn't reconize client type\n");
		exit (-1);
	}

	printf ("%s", msg);

	close (sock);

	return NULL;
}

void print_ip (char host []) {

	int s, family;
	struct ifaddrs *ifa, *ifaddr;


	if (getifaddrs(&ifaddr) == -1)
	{
		perror ("getifaddrs");
		exit (-1);
	}


	ifa = ifaddr;
         while (ifa != NULL)
	{
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
		   if (strcmp (host, "127.0.0.1"))
		   {
	                   printf("\t@IP: %s\n", host);
		   }
               }
		ifa = ifa->ifa_next;
           }
}

int create_msg_queue () {

	int msqid = -1;
	key_t key = 1;

	if ((msqid = msgget(ftok ("/tmp", key), IPC_CREAT)) == -1)
	{
		perror ("msgget");
		exit (-1);
	}

	return msqid;
}

void write_msg (int msqid, char msg []) {

	int lent = sizeof msg;
	if (msgsnd(msqid, msg, lent, 0) == -1)
	{
		perror ("msgsnd");
		exit (-1);
	}
}

void read_msg (int msqid) {

	char message [50];
	if (msgrcv(msqid, message, sizeof message, 0, MSG_NOERROR) == -1)
	{
		perror ("msgrcv");
		exit (-1);
	}

	printf ("%s", message);
}
