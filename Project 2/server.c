#include <pthread.h>
#include "header.h"

int client, fd;
int confirm_msg;
struct email* tmp;
int port = 0;

Users* ini = NULL;
Users* fim = NULL;

pthread_t mythreads[THREADS];
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER, thread_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]){
	
	if (argc == 3) {
		port = atoi(argv[2]);
		printf("Port %d\n", port);
	}

	struct sockaddr_in client_addr;
	int client_addr_size;

	signal(SIGINT, end);

	printf("Servidor a iniciar:\n");
	serverStart();

	load_users("userpass.txt");

	while(1){

		client_addr_size = sizeof(client_addr);
		client = accept(fd,(struct sockaddr *)&client_addr,&client_addr_size);

		if(client > 0){

				pthread_mutex_unlock(&client_mutex);
			
				pthread_mutex_lock(&thread_mutex);
		}
	}

	return 0;

}

void erro(char *msg){

	printf("Erro: %s\n",msg);
	exit(-1);
}

void end(int signum){

	printf("A desligar servidor\n");
	close(fd);
	fflush(stdout);
	exit(0);
}

void serverStart(){

	int i;
	struct sockaddr_in addr;
	bzero((void *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(port == 0){
		addr.sin_port = htons(SERVER_PORT);
	}
	else
		addr.sin_port = htons(port);
	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		erro("na funcao socket");
	if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
		erro("na funcao bind");
	if( listen(fd, 5) < 0)
		erro("na funcao listen");

	int id[THREADS];

	pthread_mutex_lock(&client_mutex);
	pthread_mutex_lock(&thread_mutex);

	for (i = 0; i < THREADS; i++){
		id[i] = i;
		pthread_create(&mythreads[i], NULL, (void*) &worker, &id[i]);
	}
}

void worker(int id){

	int client_fd;
	Users* node = NULL;
	request_server req;

	while(1){

		pthread_mutex_lock(&client_mutex);
		client_fd = client;
		pthread_mutex_unlock(&thread_mutex);

		printf("Server recieved request %d\n", client_fd);
		int nread = 0;
		nread = read(client_fd, &req, sizeof(request_server));

		int confirm = 0;

		node = login(req.username, req.password);

		if(node != NULL){

			printf("Logged In\n");
			confirm = 0;
			write(client_fd, &confirm, sizeof(int));
			server_menu(node, client_fd);
		}
		else {
				printf("Failed Logging in\n");
				confirm = 1;
				write(client_fd, &confirm, sizeof(int));
			}
		fflush(stdout);	
		close(client_fd);

	}
	
}

Users* login(char* username, char* password){

	Users* aux = ini;
	while(aux != NULL){

		if((!strcmp(aux->username, username)) && (!strcmp(aux->password, password))){
			return aux;
		}

		aux = aux->next;
	}
	return NULL;
}

void load_users (char *txt){ //FALTA GRUPO

	FILE *fich;
	fich = fopen(txt, "r");
	int inicio = 1;

	if(fich == NULL){
		printf("Error opening the fille\n");
		return;
	}

	char buffer[30];
	char *s = ",", *nametmp, *passtmp; //*grupotmp;

    while(fgets(buffer,sizeof(buffer),fich) != NULL)
    {
    	nametmp = strtok(buffer, s);
    	passtmp = strtok(NULL, s);	
    	printf("Load User %s\n",nametmp);
    	passtmp[strlen(passtmp)-1] = '\0';

    	create_user(nametmp, passtmp);

    	if( inicio == 1)
    	{
    		ini = fim ;
    		inicio = 0;
    	}
    }
    fclose(fich);
}

void create_user(char *usertmp, char *passtmp){

	Users* aux = (Users*) malloc(sizeof(Users));
	strcpy(aux->username, usertmp);
	strcpy(aux->password, passtmp);
	aux->numemails = 0;
	aux->oper = 0;
	aux->grupo = 1;
	aux->listaEmails = NULL;
	aux->next = NULL;

	if(fim != NULL){
		fim->next = aux;
	}

	fim = aux;
}

void server_menu(Users* node, int client_fd){

	int option = 0;
	while(1)
	{
		printf("MENU SERVIDOR\n");
		read(client_fd, &option, sizeof(int));
		printf("Opção: %d\n", option);
		if(option == 1){
			server_listmess(node, client_fd);
		} else if(option == 2) {
			server_listusers(node, client_fd);
		} else if(option == 3) {
			server_sendmess(node, client_fd);
		} else if(option == 4) {
			server_listmess(node, client_fd);
		} else if(option == 5) {
			server_removemes(node, client_fd);
		} else if(option == 6) {
			server_changepass(node, client_fd);
		} else if(option == 7) {
			server_oper(node, client_fd);
		} else if(option == 8)	{
			printf("Cliente saiu\n");
			break;
		} else if(option == 9) {
			server_openmsg(node, client_fd);
		} else if(option == 10) {
			server_createacc(node, client_fd);	
		} else if(option == 11) {
			server_deluser(node, client_fd);		
		} else {
			printf("Opção não encontrada\n");
			break;
		}
	}

}

void server_listmess(Users* node, int client_fd){

	printf("Mostrar Emails\n");

	int i;
	write(client_fd, &node->numemails, sizeof(int));
	struct email* email_aux = node->listaEmails;
	for ( i = 0; i < node->numemails; i++)
	{
		write(client_fd, email_aux, sizeof(struct email));
		email_aux = email_aux->next;
	}
}

void server_listusers(Users* node, int client_fd){

	printf("Listar Utilizadores\n");

	Users* aux = node;
	while(aux != NULL){

		printf("user %s\n", aux->username);

		aux = aux->next;
	}
	
}

void server_sendmess(Users* node, int client_fd){

	printf("Envio de Email\n");

	struct email novo, *aux;

	Users* dest = NULL;
	int confirm = 1, send = 0;
	read(client_fd, &send, sizeof(int));
	while(send== 1){

		read(client_fd, &novo, sizeof(struct email));
		aux = (struct email*)malloc(sizeof(struct email));

		strcpy(aux->sender, novo.sender);
		strcpy(aux->reciever, novo.reciever);
		strcpy(aux->assunto, novo.assunto);
		strcpy(aux->msg, novo.msg);

		printf("Mensagem a enviar %s\n", novo.reciever);

		dest = checkDest(novo.reciever);

		if(dest != NULL)
		{
			printf("Destinatario Encontrado\n");
			confirm = 0;
			write(client_fd, &confirm, sizeof(int));
			organize(dest, aux);
		}
		else
		{
			tmp = aux;
			confirm = confirm_msg;
			write(client_fd, &confirm, sizeof(int));
		}
		read(client_fd, &send, sizeof(int));
	}

}

Users* checkDest(char* to)
{
	Users* aux = ini;
	while ( aux != NULL)
	{
		printf("Destinatario %s\n", aux->username);

		if(strcmp(aux->username, to) == 0){
			return aux;
		}
		aux = aux->next;
	}

	return NULL;
}

void organize(Users* node, struct email *novo)
{
	node->numemails++;
	node->lastnum++;
	novo->nemail = node->lastnum;
	if(node->listaEmails == NULL)
	{
		novo->next = NULL;
		novo->prev = NULL;
		node->listaEmails = novo;
	}
	else
	{
		struct email *aux = node->listaEmails;
		while(aux->next != NULL)
			aux = aux->next;
		novo->prev = aux;
		aux->next = novo;
	}
}

void server_openmsg(Users* node, int client_fd)
{
	printf("Abrir novo Email\n");
	int nemail;
	read(client_fd, &nemail, sizeof(int));
	struct email* email = NULL;
	email = server_findEmail(nemail, node);
	int confirm;
	if( email != NULL)
	{
		email->read = 1;
		confirm = 0;
		write(client_fd, &confirm, sizeof(int ));
		struct email aux;
		aux.nemail = email->nemail;
		strcpy(aux.sender, email->sender);
		strcpy(aux.reciever, email->reciever);
		strcpy(aux.assunto, email->assunto);
		strcpy(aux.msg, email->msg);

		write(client_fd, &aux, sizeof(struct email));
	}
	else
	{
		confirm = 1;
		write(client_fd, &confirm, sizeof(int ));
	}
}

struct email* server_findEmail(int num, Users* node ){

	struct email *aux = node->listaEmails;
	while(aux != NULL)
	{
		if(aux->nemail == num)
			return aux;
		aux = aux->next;
	}
	return NULL;
}

void server_removemes(Users* node, int client_fd){

	printf("Apagar Email\n");

	int num = 0, confirm = 1;

	read(client_fd, &num, sizeof(int));
	struct email *aux = node->listaEmails;
	
	while(aux != NULL)
	{
		if(aux->nemail == num) {
			if(aux->prev!= NULL){
				(aux->prev)->next = aux->next;
			}
			else
				node->listaEmails = aux->next;

			if(aux->next != NULL){
				(aux->next)->prev = aux->prev;
			}

			confirm = 0;

			node->numemails--;

			free(aux);
			break;
		}

		aux = aux->next;
	}

	write(client_fd, &confirm, sizeof(int));
}

void server_changepass(Users* node, int client_fd){

	printf("Mudar Password\n");
	int nread, confirm;
	char password[20];
	nread = read(client_fd,password,21+1);
	password[nread]='\0';

	strcpy(node->password, password);
	printf("Nova Password: %s\n", node->password);

	confirm = 0;
	write(client_fd, &confirm, sizeof(int));

}

void server_oper(Users* node, int client_fd){

	printf("Pedido de atribuição de operador\n");
	int confirm;
	request_server aux;
	int nread = 0;
	nread = read(client_fd, &aux, sizeof(request_server));

	if((strcmp(aux.username,"su") == 0) && (strcmp(aux.password,"su") == 0)){

		printf("Novo operador\n");
		confirm = 0;
		write(client_fd, &confirm, sizeof(int));
		node->oper = 1;
		
	} else
		printf("Autenticação falhou\n");
		confirm = 1;
		write(client_fd, &confirm, sizeof(int));
		

}

void server_createacc (Users* node, int client_fd){

	int confirm;

	if(node->oper == 1){

		confirm = 0;
		write(client_fd, &confirm, sizeof(int));

		request_server aux;
		int nread = 0;
		nread = read(client_fd, &aux, sizeof(request_server));
		printf("user %s\n", aux.username);
		create_user(aux.username, aux.password);
		printf("Conta Criada\n");

	} else
		confirm = 1;
		write(client_fd, &confirm, sizeof(int));
		printf("Não é operador\n");



}

void server_deluser(Users* node, int client_fd){



}