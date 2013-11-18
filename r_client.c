#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


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
/*
	//Recherche du protocole
	struct protoent* protocol;
	protocol = getprotobyname("tcp");

	//Recherche du service
	struct servent* service;
	service = getservbyname("kermit", protocol -> p_name);

	//Recherche du nom de l'hôte
	struct hostent* host;
	host = gethostbyname("localhost");

	//Initialisation de la structure sockaddr_in
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = service -> s_port;
	addr.sin_addr.s_addr = ((struct in_addr*)(host -> h_addr)) -> s_addr;

	//Nommage
	int code;
	code = bind(sock, (struct sockaddr*) & addr, sizeof(struct sockaddr_in));
	if(code == -1)
	{
		perror("bind");
		exit(1);
	}
	else printf("Bind success\n");
*/
	//Définition du serveur
struct hostent *hostinfo = gethostbyname (args[1]);
	struct sockaddr_in server = {0};
	//memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(*(int *)args[2]);
	server.sin_addr = *(struct in_addr *) hostinfo->h_addr;
	//inet_aton(args[1], &server.sin_addr);

	//On se connecte au serveur
	printf("Trying to connect to %s %s\n", args[1], args[2]);
	if(connect(sock, (struct sockaddr*)&server, sizeof(server)) == -1) {
		perror("connect");
		close(sock);
		exit (-1);
	}
	else printf("Connection succeed\n");

	//On attend un message de la part du serveur
	char buffer [50];
	
	//if(listen(sock, 1) == 0)
	while(strlen(buffer) <= 0)
	{
		int new;
		struct sockaddr_in servaddr;
		socklen_t len;
		new = accept(sock, (struct sockaddr*)&servaddr, &len);

		recv(new, buffer, sizeof(buffer), 0);
		printf("buffer = %s\n", buffer);

	}
	//else { perror("listen"); return 1; }
		
	close(sock);

	

	return 0;

}
