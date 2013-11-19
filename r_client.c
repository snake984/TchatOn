#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

void Exit(int sig, int sock);


int main(int argc, char** args)
{
	if(args[1] == NULL)
	{
		printf("You must enter an IP address and a number of port\n");
		return 0;
	}

	if(args[2] == NULL)
	{
		printf("You must enter an IP address and a number of port\n");
		return 0;
	}

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//On interprète le signal SIGINT (Ctrl+C)
	struct sigaction exitAction;
	exitAction.sa_handler = Exit;
	sigaction(SIGINT, &exitAction, NULL);
	
	//Définition du serveur

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi (args[2]));
	inet_aton(args[1], &server.sin_addr);

	//On se connecte au serveur
	printf("Trying to connect to %s %s\n", args[1], args[2]);
	if(connect(sock, (struct sockaddr*)&server, sizeof(server)) == -1) {
		perror("connect");
		close(sock);
		exit (-1);
	}
	else printf("Connection succeed\n");

	//On attend un message de la part du serveur

	if (send(sock, "R", 10, 0) == -1)
	{
		perror ("send");
		exit (-1);
	}

	char buffer[50];
	int i;

	while(1)
	{
		if((i = recv(sock, buffer, sizeof buffer, 0)) == -1)
		{
			printf ("Server disconnected\n");
			break;
		}
		if (i==0)
		{
			break;
		}

		if (send(sock, "Ok", 5, 0) == -1)
		{
			perror ("send");
			exit (1);
		}

printf("%s", buffer);
	}
		
	close(sock);

	return 0;

}

void Exit(int sig, int sock)
{
	printf("Exit client \n");
	if(send(sock, "Disconnect", 15, 0) == -1)
	{
		perror("send");
		exit(1);
	}

	close(sock);
	exit(0);
}
