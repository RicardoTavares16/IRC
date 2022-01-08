#include "header.h"

int fd;
char user[LOGINSIZE], pass[20];

int main(int argc, char *argv[]){

	signal(SIGINT, end);

	// Socket initialization
	fd = init_client(argc, argv);

	start();

	return 0;

}

void erro(char *msg){
	printf("Erro: %s\n",msg);
	exit(-1);
}

void end(int signum){

	printf("Turning off\n");
	close(fd);
	exit(0);
}

int init_client(int argc, char *argv[]){

	char endServer[100];
	int fd;
	struct sockaddr_in addr, serv_addr;
	struct hostent *hostPtr;
	

	if (argc != 3) {
		printf("cliente <host> <port> \n");
		exit(-1);
	}

	strcpy(endServer, argv[1]);
	
	if ((hostPtr = gethostbyname(endServer)) == 0) erro("Fail getting the adress");
	
	bzero((void *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
	addr.sin_port = htons((short) atoi(argv[2]));
	
	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1) erro("socket");
	
	if( connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0) erro("Connect");
	return fd;

}

void start(){

	printf("Autenticação\n");
	printf("Username:\n");
	scanf("%s", user);
	printf("Password:\n");
	scanf("%s", pass);

	request_server req;
	strcpy(req.username, user);
	strcpy(req.password, pass);

	write(fd, &req, sizeof(request_server));

	int confirm;

	read(fd, &confirm, sizeof(int));

	if(confirm == 0){

		printf("Logged In\n");
		menu();

	} else {

		printf("Login Error\n");
	  }
}

void menu(){

	int option; //para passar como integer
	while(1) {
		printf("\n  -  Menu (Escolher opção)  -  \n\n");
		printf("1)   LIST_MESS – para listar todas as mensagens por ler.\n");
		printf("2)   LIST_USERS – para listar todos os clientes autorizados.\n");
		printf("3)   SEND_MESS – para enviar uma mensagem para um cliente (autorizado).\n");
		printf("4)   LIST_READ – para listar todas as mensagens já lidas.\n");
		printf("5)   REMOVE_MES – para apagar mensagens.\n");
		printf("6)   CHANGE_PASSW – alterar a password\n");
		printf("7)   OPER – para o cliente obter os privilégios do operador.\n");
		printf("8)   QUIT – para o cliente abandonar o sistema.\n");
		printf("9)   OPEN_MSG – abrir mensagem específica.\n");
		printf("10)   CREATE_ACC – (OPER) Criar conta.\n");
		printf("11)   DEL_USER – (OPER) Apagar utilizador da base dados.\n");
		printf("Opção: ");
		scanf("%d", &option);
	
		write(fd, &option, sizeof(int));

		if(option == 1){
			listmess();
		} else if(option == 2) {
			listusers();
		} else if(option == 3) {
			sendmess();
		} else if(option == 4) {
			listread();
		} else if(option == 5) {
			removemes();
		} else if(option == 6) {
			changepass();
		} else if(option == 7) {
			oper();
		} else if(option == 8) {
			printf("Quit\n");
			break;
		} else if(option == 9) {
			openmsg();	
		} else if(option == 10) {
			createacc();	
		} else if(option == 11) {
			deluser();			
		} else printf("\n\nOPTION NOT FOUND\n\n");

		

	}

	printf("\nBye!\n");
	close(fd);
	exit(0);

}

void listmess(){

	printf("Listagem de Emails por ler\n");

	int i,  numemail = 0;
	struct email aux;

	read(fd, &numemail, sizeof(int) );
	for (i = 0 ; i < numemail; i++)
	{
		read(fd, &aux, sizeof(struct email));
		if(aux.read != 1){

			printf("Ńº: %d; De: %s; Assunto: %s; \n",aux.nemail,  aux.sender, aux.assunto );
		}
	}
}

void listusers(){ //Server imprime



}

void sendmess(){

	printf("Novo Email\n");
	struct email novo ;
	strcpy(novo.sender,user);

	printf("Destinatário:");
	char dest [MSGSIZE], *buffer, *s = " \n";
	dump_line(stdin);
	fgets (dest, sizeof(dest), stdin);


	printf("Assunto:");
	scanf("%s",novo.assunto);

	printf("Mensagem:\n");
	dump_line(stdin);
	fgets (novo.msg, sizeof(novo.msg), stdin);

	int confirm, send = 1;
	buffer = strtok(dest, s);
	while(buffer != NULL)
    {

    	strcpy(novo.reciever, buffer);
    	write(fd, &send, sizeof(int));
    	write(fd,&novo, sizeof(struct email));
		confirm = 1;
		read(fd, &confirm, sizeof(int));
		if ( confirm == 0)
			printf("Email Enviado para %s\n", buffer);
		else
			printf("Email não enviado %s\n", buffer);	
		buffer = strtok(NULL, s);	
    }
    send = 0;
    write(fd, &send, sizeof(int));

}

void dump_line(FILE* fp)
{
  int ch;
  while( getchar() != '\n' )
    ;

}

void openmsg()
{
	int nemail = 0, confirm;
	struct email email_aux;
	printf("Nº do Email a ler: ");
	scanf("%d", &nemail);
	write(fd, &nemail, sizeof(int));
	read(fd,&confirm, sizeof(int));
	if( confirm == 0)
	{
		printf("Email encontrado\n");
		read(fd, &email_aux, sizeof(struct email));
		printf("Remetente: %s\n", email_aux.sender);
		printf("Destinatário: %s\n", email_aux.reciever);
		printf("Assunto: %s\n", email_aux.assunto);
		printf("Email:%s\n", email_aux.msg);
	}
	else
		printf("Email não encontrado\n");
		
}

void listread(){

	printf("Listagem de Emails já lidos\n");
	int nemails = 0, i ;
	struct email email_aux;
	read(fd, &nemails, sizeof(int) );
	for ( i = 0 ; i < nemails; i++)
	{
		read(fd, &email_aux, sizeof(struct email));

		if(email_aux.read == 1){
		printf("Ńº: %d; De: %s; Assunto: %s; \n",email_aux.nemail, email_aux.sender, email_aux.assunto);
		}
	}

}

void removemes(){

	printf("Nº de Email a apagar:");
	int nemal ,confirm = 1;
	scanf("%d", &nemal);
	write(fd, &nemal, sizeof(int));
	read(fd, &confirm, sizeof(int));

	if ( confirm == 0)
		printf("Email apagado\n");
	else
		printf("Email não encontrado ");

}

void changepass(){

	int confirm;
	printf("Mudar Password\n");
	char password[20];
	printf("Password: ");
	scanf("%s", password);
	write(fd,password,strlen(password)+1);

	read(fd, &confirm, sizeof(int));

	if(confirm == 0){
		printf("Password mudada com sucesso\n");
	}
	else{
		printf("Não foi possível mudar a password\n");
	}


}

void oper(){

	printf("Previlégios de Operador\n");

	int confirm;
	char usr[20], pw[20];

	printf("Autenticação\n");
	printf("Username:\n");
	scanf("%s", usr);
	printf("Password:\n");
	scanf("%s", pw);

	request_server oper;
	strcpy(oper.username, usr);
	strcpy(oper.password, pw);

	write(fd, &oper, sizeof(request_server));

	read(fd, &confirm, sizeof(int));

	if(confirm == 0){

		printf("É agora operador\n");

	} else {

		printf("Autenticação Falhada\n");
	  }




}

void createacc(){

	int confirm;

	read(fd, &confirm, sizeof(int));

	if(confirm == 0){

		char usr[20], pw[20];
		printf("Username:\n");
		scanf("%s", usr);
		printf("Password:\n");
		scanf("%s", pw);

		request_server oper;
		strcpy(oper.username, usr);
		strcpy(oper.password, pw);

		write(fd, &oper, sizeof(request_server));



	} else {
		printf("Não é operador\n");
	}


}

void deluser(){

	
}

