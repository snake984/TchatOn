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
#include <time.h>
#include <signal.h>

#define NB_R_MAX 5
#define NB_W_MAX 3
#define SIZE_MAX_MSG 1000
#define SIZE_MAX_PSEUDO 25

struct message {
	long type;
	char buffer [SIZE_MAX_MSG];
	char pseudo [SIZE_MAX_PSEUDO];
}message;


int create_server ();
void *client (void *arg);
void print_ip (char ip []);
int create_msg_queue ();
struct message read_msg (int msgqid, int num_r);
void write_msg (int msgqid, struct message msg);
void c_reader (int sock, int num_r);
void c_writer (int sock, char pseudo []);
void create_writer (int argc, char** args);
void create_reader (int argc, char** args);
void exit_reader(int sig);
void exit_server (int sig);


int KEY = 1;
int MSGQID = -1;
int NB_R = 0;
int TAB_R_CONNECTED [NB_R_MAX] = {0};
char *TAB_W_PSEUDO [NB_W_MAX] = {NULL};
int SERVER_SOCKET = -1;
int R_SOCKET = -1;
int CLIENT_SOCKETS [NB_R_MAX + NB_W_MAX] = {0};
