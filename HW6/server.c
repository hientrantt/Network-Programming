#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<ctype.h>
#include <signal.h>
#include <sys/wait.h>

const int STATUS_ACTIVE = 1;
const int STATUS_BLOCKED = 0;
const int STATUS_IDLE = 2;
const int ACTIVATION_CODE = 20184095;

char usernameOfAccountLogin[50];
char filename[50] = "account.txt";

typedef struct{
    char name[50];
    char password[50];
    int status;
} Account;

typedef struct Node
{
	Account account;
    int check;
	struct Node* next;
} Node;

Node* makeNewNode(Account);
int addNode(Node**, Account);
Node* getUser(Node*, char*);
int setCheckNode(Node**, char*);
int resetCheckNode(Node**, char*);
int setStatus(Node**, char*, int);
int setPassword(Node**, char*, char*);
int readFileInput(Node**);
char* trim(char*);
int xSpace(char*);
int xSpecialCharacter(char*, char*, char*);
int writeFile(Node*);

short SocketCreate(void);
int BindCreatedSocket(int, char*);

void sig_chld(int signo)
{
    pid_t pid;
    int stat;
    pid = waitpid(-1, &stat, WNOHANG );
    printf("child %d terminated\n", pid);
}

int main(int argc, char *argv[])
{
    if(argc!=2){
        printf("Invalid arguments\n");
        return 1;
    }

    int socket_desc, sock, clientLen, read_size;
    struct sockaddr_in server, client;
    char client_message[200]= {0};
    char message[100] = {0};
    const char *pMessage = "hello aticleworld.com";
    //Create socket
    socket_desc = SocketCreate();
    if (socket_desc == -1)
    {
        printf("Could not create socket");
        return 1;
    }
    printf("Socket created\n");
    //Bind
    if( BindCreatedSocket(socket_desc, argv[1]) < 0)
    {
        //print the error message
        perror("bind failed.");
        return 1;
    }
    printf("bind done\n");
    
    //get data
    Account temp;
    strcpy(temp.name, "unknown");
    strcpy(temp.password, "unknown");
    temp.status = 0;
	Node* head = makeNewNode(temp);
    Node* user;

    //Listen
    listen(socket_desc, 3);
    //Accept and incoming connection
    
    pid_t pid;
    signal(SIGCHLD, sig_chld);
    while(1)
    {
        printf("Waiting for incoming connections...\n");
        clientLen = sizeof(struct sockaddr_in);
        //accept connection from an incoming client
        sock = accept(socket_desc,(struct sockaddr *)&client,(socklen_t*)&clientLen);
        if (sock < 0)
        {
            perror("accept failed");
            return 1;
        }
        printf("Connection accepted\n");
        if((pid = fork())==0){
            close(socket_desc);
            
            int check=0;
            char numberPw[50], alphabetPw[50];
            while (1){
                memset(client_message, '\0', strlen(client_message));
                memset(message, '\0', strlen(message));
                //Receive a reply from the client
                if( recv(sock, client_message, 200, 0) < 0)
                {
                    printf("recv failed");
                    break;
                }
                
                printf("Client reply : %s\n",client_message);
                strcpy(message, client_message);

                switch (check){
                //get username and check
                case 0:
                    readFileInput(&head);
                    strcpy(temp.name, client_message);
                    trim(temp.name);
                    if(strlen(temp.name)==0){
                            check = -1;
                            strcpy(message,"byeee");
                    }else if((user=getUser(head, temp.name))==NULL){
                        strcpy(message,"cannot find account\nUsername: ");
                    }else{
                        check=1;
                        strcpy(message, "Password: ");
                    }
                    break;
                //get password and check
                case 1:
                    readFileInput(&head);
                    check = 0;
                    strcpy(temp.password, client_message);
                    trim(temp.password);
                    if(strcmp(user->account.password, temp.password)!=0){
                        if(setCheckNode(&head, temp.name)<4){
                            check=1;
                            strcpy(message,"not OK\nPassword: ");
                        }else{
                            setStatus(&head, temp.name, STATUS_BLOCKED);
                            strcpy(message,"block account\nUsername: ");
                            writeFile(head);
                            check=0;
                        }
                    }else{
                        if(user->account.status == STATUS_ACTIVE){
                            strcpy(message,"oK\nNew password: ");
                            check = 2;
                            strcpy(usernameOfAccountLogin, user->account.name);
                            writeFile(head);
                        }else if(user->account.status == STATUS_BLOCKED || user->account.status == STATUS_IDLE){
                            strcpy(message,"acount not ready\nUsername: ");
                        }
                        
                    }
                    break;
                //get new password and check
                case 2:
                    readFileInput(&head);
                    trim(client_message);
                    printf("%s\n", client_message);
                    
                    if(user->account.status == STATUS_BLOCKED || user->account.status == STATUS_IDLE){
                        strcpy(message,"acount not ready\nUsername: ");
                        check=0;
                    }else{
                        if(strcmp(client_message, "bye")==0){
                            check = 0;
                            resetCheckNode(&head, user->account.name);
                            sprintf(message,"goodbye %s\nUsername: ", temp.name);
                        }else if(xSpecialCharacter(client_message, numberPw, alphabetPw) == -1){
                            strcpy(message,"Error\nNew password: ");
                        }else{
                            if(strlen(client_message)==0){
                                check = -1;
                                strcpy(message,"byeee");
                            }else{
                                if(strlen(numberPw)!=0 && strlen(alphabetPw)==0){
                                    sprintf(message,"%s\nNew password: ", numberPw);
                                }else if(strlen(numberPw)==0 && strlen(alphabetPw)!=0){
                                    sprintf(message,"%s\nNew password: ", alphabetPw);
                                }else if (strlen(numberPw)!=0 && strlen(alphabetPw)!=0){
                                    strcat(numberPw, "\n");
                                    strcat(numberPw, alphabetPw);
                                    sprintf(message,"%s\nNew password: ", numberPw);
                                }
                                setPassword(&head, temp.name, client_message);
                                writeFile(head);
                            }
                        }
                    }

                    break;
                
                default:
                    check = -1;
                    strcpy(message,"byeee");
                    break;
                }

                // Send some data
                if( send(sock, message, strlen(message), 0) < 0)
                {
                    printf("Send failed");
                    return 1;
                }
                if(check==-1){
                    break;
                }
            }

            close(sock);
            exit(0);

        }
        
    }
    close(sock);
    exit(0);
    return 0;
}

short SocketCreate(void)
{
    short hSocket;
    printf("Create the socket\n");
    hSocket = socket(AF_INET, SOCK_STREAM, 0);
    return hSocket;
}

int BindCreatedSocket(int hSocket, char* portNumberStr)
{
    int iRetval=-1;
    // int ClientPort = 90190;
    int portNumber = atoi(portNumberStr);
    int ClientPort = portNumber;
    struct sockaddr_in  remote= {0};
    /* Internet address family */
    remote.sin_family = AF_INET;
    /* Any incoming interface */
    remote.sin_addr.s_addr = htonl(INADDR_ANY);
    remote.sin_port = htons(ClientPort); /* Local port */
    iRetval = bind(hSocket,(struct sockaddr *)&remote,sizeof(remote));
    return iRetval;
}

int readFileInput(Node** head){
    FILE *fin = fopen(filename, "r");
    FILE *ftest = fopen(filename, "r");
    if(fin==NULL || ftest==NULL){
        printf("ko mo duoc file input!\n");
        return 0;
    }
    char username[50], password[50], line[500];
    int status;
    Account account;
    while (1)
    {
        memset(line, 0, strlen(line));
        fgets(line, sizeof(char)*500, ftest);
        if(strlen(line)<6){
            break;
        }
        fscanf(fin, "%s %s %d", username, password, &status);
        strcpy(account.name, username);
        strcpy(account.password, password);
        account.status = status;

        int check=0;
        Node* temp = (Node*) malloc(sizeof(Node));
        temp = *head;
        while(temp!=NULL)
        {
            if(strcmp(temp->account.name, account.name)==0){
                strcpy(temp->account.password, account.password);
                temp->account.status = account.status;
                check=1;
                break;
            }
            temp=temp->next;
        }
        if(check==0)
            addNode(head, account);
    }
    fclose(fin);
    fclose(ftest);
}

int writeFile(Node* head){
    FILE *fout = fopen(filename, "w");
	Node* temp = (Node*) malloc(sizeof(Node));
	temp = head;
	while(temp!=NULL)
	{
		fprintf(fout, "%s %s %d\n", temp->account.name, temp->account.password, temp->account.status);
		temp=temp->next;
	}
    fclose(fout);
}

char* trim(char* key){
    if(key[strlen(key)-1]=='\n')
        key[strlen(key)-1]='\0';

    char* tmp = key;
    int l = strlen(key);
    while(isspace(tmp[l-1]))
        tmp[--l]='\0';
    while (*tmp && isspace(*tmp))
        ++tmp, --l;
    memmove(key, tmp, l+1);
}

int xSpace(char* key){
    char* tmp = key;
    for(int i=0; i<strlen(key); i++){
        if(tmp[i]==' ' || tmp[i] == '\t' || tmp[i] == '\n'){
            return -1;
        }
    }
    return 1;
}

int xSpecialCharacter(char* key, char* number, char* alphabet){
    char* tmp = key;
    memset(number, 0, strlen(number));
    memset(alphabet, 0, strlen(alphabet));
    int n=0, a=0;
    for(int i=0; i<strlen(key); i++){
        if(tmp[i]>=48 && tmp[i]<=57){
            number[n++] = tmp[i];
        }else if((tmp[i]>=65 && tmp[i]<=90)||(tmp[i]>=97 && tmp[i]<=122)){
            alphabet[a++] = tmp[i];
        }else{
            return -1;
        }
    }
    return 1;
}

Node* makeNewNode(Account account)
{
	Node* new = (Node*) malloc(sizeof(Node));
    new->account=account;
    new->check=0;
	new->next=NULL;
	return new;
}

int addNode(Node** head, Account account)
{
    if((*head)->next==NULL && strcmp((*head)->account.name, "unknown")==0 && strcmp((*head)->account.password, "unknown")==0){
        (*head)->account=account;
        return 1;
    }
	Node* temp = makeNewNode(account);
	temp->next = *head;
	*head = temp;
}

Node* getUser(Node* head, char *name)
{
	Node* temp = (Node*) malloc(sizeof(Node));
	temp = head;
	while(temp!=NULL)
	{
		if(strcmp(temp->account.name, name)==0){
            return temp;
        }
		temp=temp->next;
	}
    return NULL;
}

int setCheckNode(Node** head, char *name)
{
	Node* temp = (Node*) malloc(sizeof(Node));
	temp = *head;
	while(temp!=NULL)
	{
		if(strcmp(temp->account.name, name)==0){
            return ++(temp->check);
        }
		temp=temp->next;
	}
    return -1;
}

int resetCheckNode(Node** head, char *name){
    Node* temp = (Node*) malloc(sizeof(Node));
	temp = *head;
	while(temp!=NULL)
	{
		if(strcmp(temp->account.name, name)==0){
            temp->check=0;
        }
		temp=temp->next;
	}
    return -1;
}

int setStatus(Node** head, char *name, int status)
{
	Node* temp = (Node*) malloc(sizeof(Node));
	temp = *head;
	while(temp!=NULL)
	{
		if(strcmp(temp->account.name, name)==0){
            temp->account.status = status;
            temp->check = 0;
        }
		temp=temp->next;
	}
    return -1;
}

int setPassword(Node** head, char *name, char* newpassword)
{
	Node* temp = (Node*) malloc(sizeof(Node));
	temp = *head;
	while(temp!=NULL)
	{
		if(strcmp(temp->account.name, name)==0){
            strcpy(temp->account.password, newpassword);
            return 1;
        }
		temp=temp->next;
	}
    return -1;
}
