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

#define NB_R_MAX 5
#define NB_W_MAX 3

struct message {
	long type;
	char buffer [50];
	char pseudo [20];
}message;


void *client (void *arg);
void print_ip (char ip []);
int create_msg_queue ();
struct message read_msg (int msgqid, int num_r);
void write_msg (int msgqid, struct message msg);
void c_reader (int sock, int num_r);
void c_writer (int sock, char pseudo []);

int KEY = 1;
int MSGQID = -1;
int NB_R = 0;
int TAB_R_CONNECTED [NB_R_MAX] = {0};


int main (int argc, char *argv []) {

	
	int sock=-1;

	if ((sock = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
		perror ("socket");
		exit (-1);
	}

	char ip [50];
	print_ip (ip);

	struct sockaddr_in addr;
	inet_aton("127.0.0.1", &addr.sin_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(0);
	socklen_t sock_len = sizeof addr;
	

	if (bind (sock, (struct sockaddr*) &addr, sock_len) == -1) {
		perror ("bind");
		exit (-1);
	}

	if(listen(sock, (NB_R_MAX + NB_W_MAX)) == -1)
	{
	    perror("listen");
	    exit(1);
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

	MSGQID = create_msg_queue ();


	int c_sock = -1;
	while (1) {
		struct sockaddr_in c_addr;
		socklen_t c_addr_size = sizeof c_addr;
	
		if ((c_sock = accept(sock, (struct sockaddr *) &c_addr,
				     &c_addr_size))
			 == -1)
		{
			perror ("accept\n");
			exit (1);
		}
		else
		{
			pthread_t *t = malloc (sizeof (pthread_t));

			if (pthread_create(t, NULL, &client,
					   (void *) &c_sock) 
				!= 0) 
			{
				perror ("pthread_create");
				exit (1);
			}
		}
printf ("Client trying to connect...\n");
	}

	close (sock);

	return 0;
}

void *client (void *arg) {

	int sock = *((int*)arg);
	char buffer [50];

	if (recv(sock, buffer, sizeof buffer, 0) == -1)
	{
		perror ("recv");
		exit (-1);
	}

	char *save [1];
	char *type_client = strtok_r (buffer, " ", save);
	if (strcmp (type_client, "W") == 0)
	{
		char *pseudo = strtok_r (NULL, " ", save);
		printf ("%s connected\n", pseudo);
		c_writer (sock, buffer);
	}
	else if (strcmp (type_client, "R") == 0)
	{
		NB_R++;

		if (NB_R > NB_R_MAX) {
			printf ("Too many Readers\n");
			close (sock);
			return NULL;
		}

		int i;
		for (i=0; i<NB_R_MAX; i++)
		{
			if (TAB_R_CONNECTED [i] == 0)
			{
				TAB_R_CONNECTED [i] = 1;
				break;
			}
		}

		int num_r = i+1;
		printf ("Reader %d connected\n", num_r);
		c_reader (sock, num_r);
	}
	else
	{
		printf ("Couldn't reconize client type : %s\n", type_client);
		exit (-1);
	}

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

	int msgqid = -1;
	key_t key = KEY;

	if ((msgqid = msgget(ftok ("/tmp", key), IPC_CREAT)) == -1)
	{
		perror ("msgget");
		exit (-1);
	}

	return msgqid;
}

void write_msg (int msgqid, struct message msg) {

//printf ("Message writing : %sType : %ld\n", msg.buffer, msg.type);
	if (msgsnd(msgqid, &msg, sizeof msg.buffer, 0) == -1)
	{
		perror ("msgsnd");
		exit (-1);
	}
//printf ("Done !\n");
}

struct message read_msg (int msgqid, int num_r) {

	struct message msg;
printf ("Sending message to client\n");
	if (msgrcv(msgqid, &msg, sizeof msg.buffer, num_r, 0) == -1)
	{
		perror ("msgrcv");
		exit (2);
	}

	return msg;
}

void c_reader (int sock, int num_r) {

	struct message msg;

	while (1) {
		msg = read_msg (MSGQID, num_r);

		if (send(sock, msg.buffer, sizeof msg.buffer, 0) == -1)
		{
			perror ("send");
			exit (1);
		}

		if(recv(sock, msg.buffer, sizeof msg.buffer, 0) == -1)
		{
			
			printf ("Reader %d disconnected\n", num_r);
			return;
		}

		if (strcmp(msg.buffer, "Ok") != 0)
		{
			TAB_R_CONNECTED [num_r-1] = 0;
			NB_R--;
			printf ("Reader %d disconnected\n", num_r);
			return;
		}
	}
}

void c_writer (int sock, char pseudo []) {

	struct message msg;
	int test, i;

	while(1)
	{
		if((test = recv(sock, msg.buffer, sizeof msg.buffer, 0)) == -1)
		{
			printf ("Writer disconnected\n");
			return;
		}

		if (test==0)
		{
			break;
		}

//printf ("Message received : %s", msg.buffer);

		if (NB_R == 0)
		{
			printf ("No client connected\n");
		}
printf ("NB_R : %d\n", NB_R);
		for (i=0; i<NB_R; i++) {
printf ("%d : %d", i+1, TAB_R_CONNECTED [i]);
			if (TAB_R_CONNECTED [i])
			{
				msg.type = i+1;
				write_msg (MSGQID, msg);
			}
		}
	}	
}
