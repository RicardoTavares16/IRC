#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>

#define BUF_SIZE 1024
#define LOGINSIZE 9
#define SERVER_PORT 9000
#define MSGSIZE 250
#define STRSIZE 30
#define THREADS 5


typedef struct request_server {
	char username[LOGINSIZE];
	char password[20];
} request_server;

struct email {
   int read;
   int nemail;
   char sender[LOGINSIZE];
   char reciever[LOGINSIZE];
   char assunto[STRSIZE];
   char msg[MSGSIZE];
   struct email* next;
   struct email* prev;
};

struct userNode {
   char username[LOGINSIZE];
   char password[20];
   int numemails;
   int lastnum;
   int grupo;
   int oper;
   struct email* listaEmails;
   struct userNode* next;
};

typedef struct userNode Users;


void end (int signum);
void erro(char *msg);

//Cliente
int init_client(int argc, char *argv[]);
void start();
void menu();
void listmess();
void listusers();
void sendmess();
void listread();
void removemes();
void changepass();
void oper();
void dump_line(FILE* fp);
void openmsg();
void createacc();
void deluser();



//Servidor
void serverStart();
void worker();
Users* login(char* username, char* password);
void load_users (char* txt);
void create_user(char* usertmp, char *passtmp);
void server_menu(Users* node, int client_fd);
void server_listmess(Users* node, int client_fd);
void server_listusers(Users* node, int client_fd);
void server_sendmess(Users* node, int client_fd);
void server_removemes(Users* node, int client_fd);
void server_changepass(Users* node, int client_fd);
void server_oper(Users* node, int client_fd);
Users* checkDest(char* to);
void organize(Users* u_node, struct email* novo);
void server_openmsg(Users* u_node, int client_fd);
struct email* server_findEmail(int num, Users* node);
void server_createacc (Users* node, int client_fd);
void server_deluser(Users* node, int client_fd);
