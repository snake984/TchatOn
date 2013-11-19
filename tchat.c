#include "tchat.h"

int main (int argc, char **args) {

	if (argc == 2 && strcmp (args[1], "-s") == 0)
	{
		create_server();
	}
	else if (argc == 4 && strcmp (args[1], "-r") == 0)
	{
		create_reader (argc, args);
	}
	else if (argc == 5 && strcmp (args[1], "-w") == 0)
	{
		create_writer (argc, args);
	}
	else
	{
		printf ("Wrong argument !\n");
	}

	return 0;
}

int create_server () {

	struct sigaction exitAction;
	exitAction.sa_handler = exit_server;
	sigaction(SIGINT, &exitAction, NULL);
	
	int sock=-1;

	if ((sock = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
		perror ("socket");
		exit (-1);
	}

	SERVER_SOCKET = sock;

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
			perror ("accept");
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
	}

	close (sock);

	return 0;
}

void *client (void *arg) {

	int i;
	int sock = *((int*)arg);
	char buffer [SIZE_MAX_MSG];

	if (recv(sock, buffer, sizeof buffer, 0) == -1)
	{
		perror ("recv");
		exit (-1);
	}

	char *save [1];
	char *type_client = strtok_r (buffer, " ", save);
	if (strcmp (type_client, "W") == 0)
	{
		int num_w = -1;
		char *pseudo = strtok_r (NULL, " ", save);
		printf ("%s connected\n", pseudo);

		for (i=0; i<NB_W_MAX; i++)
		{
			if (TAB_W_PSEUDO [i] != NULL)
			{
				if (strcmp (TAB_W_PSEUDO [i], pseudo) == 0)
				{
					if (send(sock, "Pseudo already exist\n", 50, 0) == -1)
					{
						perror ("send");
						exit (1);
					}
					close (sock);
					return NULL;
				}
			}
			else
			{
				num_w = i;
			}
		}

		if (num_w == -1)
		{
			if (send(sock, "Too many Writers\n", 50, 0) == -1)
			{
				perror ("send");
				exit (1);
			}
		}
		else
		{
			if (send(sock, "Accepted", 50, 0) == -1)
			{
				perror ("send");
				exit (1);
			}

			CLIENT_SOCKETS [NB_R_MAX + num_w] = sock;
			TAB_W_PSEUDO [num_w] = pseudo;

			c_writer (sock, pseudo);

			CLIENT_SOCKETS [NB_R_MAX + num_w] = -1;
			TAB_W_PSEUDO [num_w] = NULL;
		}
	}
	else if (strcmp (type_client, "R") == 0)
	{
		int num_r = -1;
		NB_R++;

		for (i=0; i<NB_R_MAX; i++)
		{
			if (TAB_R_CONNECTED [i] == 0)
			{
				num_r = i+1;
				CLIENT_SOCKETS [i] = sock;
				TAB_R_CONNECTED [i] = 1;
				break;
			}
		}

		if (num_r == -1)
		{
			if (send(sock, "Too many Readers\n", 50, 0) == -1)
			{
				perror ("send");
				exit (1);
			}
		}
		else
		{
			printf ("Reader %d connected\n", num_r);
			if (send(sock, "Accepted", 50, 0) == -1)
			{
				perror ("send");
				exit (1);
			}
			c_reader (sock, num_r);
		}
	}
	else
	{
		if (send(sock, "Couldn't reconize client type\n", 50, 0) == -1)
		{
			perror ("send");
			exit (1);
		}

		printf ("Couldn't reconize client type : %s\n", type_client);
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
                       exit(1);
                   }
		   if (strcmp (host, "127.0.0.1"))
		   {
	                   printf("\t@IP: %s\n", host);
		   }
               }
		ifa = ifa->ifa_next;
           }
	free (ifaddr);
}

int create_msg_queue () {

	int msgqid = -1;
	key_t key = KEY;

	if ((msgqid = msgget(ftok (".", key), 0666 | IPC_CREAT)) == -1)
	{
		perror ("msgget");
		exit (-1);
	}

	return msgqid;
}

void write_msg (int msgqid, struct message msg) {

//printf ("Message writing : %sType : %ld\n", msg.buffer, msg.type);
	char tmp [SIZE_MAX_MSG] = {'\0'};

	time_t timestamp = time(NULL);
	struct tm *t = localtime (&timestamp);

	sprintf (tmp, "%02u:%02u:%02u %s a dit : %s", t->tm_hour, t->tm_min, t->tm_sec, msg.pseudo, msg.buffer);
	strcpy (msg.buffer, tmp);

	if (msgsnd(msgqid, &msg, sizeof msg.buffer, 0) == -1)
	{
		perror ("msgsnd");
		exit (-1);
	}
//printf ("Done !\n");
}

struct message read_msg (int msgqid, int num_r) {

	struct message msg;

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
			
			perror ("R recv");
			return;
		}

		if (strcmp(msg.buffer, "Ok") != 0)
		{
			TAB_R_CONNECTED [num_r-1] = 0;
			CLIENT_SOCKETS [num_r-1] = -1;
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
			perror ("W recv");
			return;
		}

		if (test==0)
		{
			printf ("%s disconnected\n", pseudo);
			break;
		}

//printf ("Message received : %s", msg.buffer);

		if (NB_R == 0)
		{
			printf ("No client connected\n");
		}

		for (i=0; i<NB_R_MAX; i++) {

			if (TAB_R_CONNECTED [i])
			{
				msg.type = i+1;
				strcpy (msg.pseudo, pseudo);
				write_msg (MSGQID, msg);
			}
		}
	}	
}

void create_writer (int argc, char** args)
{
	if(args[2] == NULL)
	{
		printf("You must enter an IP address\n");
		return;
	}

	if(args[3] == NULL)
	{
		printf("You must enter a number of port\n");
		return;
	}

	if(args[4] == NULL)
	{
		printf("You must enter a pseudo\n");
		return;
	}

	if (strlen (args[4]) >= SIZE_MAX_PSEUDO)
	{
		printf ("Pseudo too long\n");
		return;
	}

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Définition du serveur

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi (args[3]));
	inet_aton(args[2], &server.sin_addr);

	//On se connecte au serveur
	printf("Trying to %s %s\n", args[2], args[3]);
	if(connect(sock, (struct sockaddr*)&server, sizeof(server)) == -1) 
	{
		perror("connect");
		close(sock);
		exit (-1);
	}
	printf("Connected to %s %s\n", args[2], args[3]);

	char identity [SIZE_MAX_PSEUDO] = {'\0'};
	sprintf (identity, "W %s", args[4]);

	if (send(sock, identity, sizeof identity, 0) == -1)
	{
		perror ("send");
		exit (-1);
	}

	char buffer [SIZE_MAX_MSG];
	if(recv(sock, buffer, sizeof buffer, 0) == -1)
	{
		perror ("recv");
		close (sock);
		return;
	}

	if (strcmp(buffer, "Accepted") != 0)
	{
		printf ("%s\n", buffer);
		close (sock);
		return;
	}

	while(1)
	{
		fgets (buffer, sizeof buffer, stdin);

		if (strcmp (buffer, "exit\n") == 0) {
			break;
		}

		if(send(sock, buffer, sizeof buffer, 0) == -1)
		{
			perror ("send");
			exit (-1);
		}
	}
		
	close(sock);
}

void create_reader (int argc, char** args)
{
	if(args[2] == NULL)
	{
		printf("You must enter an IP address\n");
		return;
	}

	if(args[3] == NULL)
	{
		printf("You must enter a number of port\n");
		return;
	}

	 R_SOCKET = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//On interprète le signal SIGINT (Ctrl+C)
	struct sigaction exitAction;
	exitAction.sa_handler = exit_reader;
	sigaction(SIGINT, &exitAction, NULL);
	
	//Définition du serveur

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi (args[3]));
	inet_aton(args[2], &server.sin_addr);

	//On se connecte au serveur
	if(connect(R_SOCKET, (struct sockaddr*)&server, sizeof(server)) == -1) {
		perror("connect");
		close(R_SOCKET);
		exit (1);
	}

	if (send(R_SOCKET, "R", 10, 0) == -1)
	{
		perror ("send");
		exit (1);
	}

	char buffer[1000] = {'\0'};
	if(recv(R_SOCKET, buffer, sizeof buffer, 0) == -1)
	{
		perror ("recv");
		close (R_SOCKET);
		return;
	}

	if (strcmp(buffer, "Accepted") != 0)
	{
		printf ("%s\n", buffer);
		close (R_SOCKET);
		return;
	}

	int i;
	while(1)
	{
		if((i = recv(R_SOCKET, buffer, sizeof buffer, 0)) == -1)
		{
			printf ("Server disconnected\n");
			break;
		}
		if (i==0)
		{
			break;
		}

		if (send(R_SOCKET, "Ok", 5, 0) == -1)
		{
			perror ("send");
			close (R_SOCKET);
			exit (1);
		}

		printf("%s", buffer);
	}
		
	close(R_SOCKET);
}

void exit_reader(int sig)
{
	printf("Exit client\n");
	if(send(R_SOCKET, "Disconnect", 15, 0) == -1)
	{
		perror("send");
		exit(1);
	}

	close(R_SOCKET);
	exit(0);
}

void exit_server (int sig) {

	int i;
	printf ("Server disconnected\n");

	for (i=0; i<(NB_R_MAX + NB_W_MAX); i++)
	{
		if (CLIENT_SOCKETS [i] > 0)
		{
			close (CLIENT_SOCKETS [i]);
		}
	}

	close (SERVER_SOCKET);

	exit (0);
}
