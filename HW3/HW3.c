#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<netinet/in.h> 
#include<netdb.h>
#include<arpa/inet.h>
#include<string.h>

const int STATUS_ACTIVE = 1;
const int STATUS_BLOCKED = 0;
const int STATUS_IDLE = 2;
const int ACTIVATION_CODE = 20184095;

char usernameOfAccountLogin[50];
char filename[50] = "account.txt";
char filenameLogin[50] = "login.txt";

typedef struct{
    char name[50];
    char password[50];
    int status;
    char homepage[200];
} Account;

typedef struct Node
{
	Account account;
    int check;
	struct Node* next;
} Node;

//Linked List
Node* makeNewNode(Account);
int addNode(Node**, Account);
Node* getUser(Node*, char*);
int setCheckNode(Node**, char*);
int setStatus(Node**, char*, int);
int setPassword(Node**, char*, char*);

//vào ra file
int readFileInput(Node**);
int writeFile(Node*);

//xử lý input
char* trim(char*);
int xSpace(char*);

//xử lý domain name | ip address
struct hostent* getInfoByNameOrIP(char*);
int printInfoName(struct hostent*);
int printInfoIP(struct hostent*);

//các chức năng
int registerAccount(Node**);
int activateAccount(Node**);
int signInAccount(Node**);
int searchAccount(Node*);
int signOutAccount(Node*);
int changePassword(Node**);
int HomepageWithDomainName(Node*);
int HomepageWithIPAddress(Node*);

int main()
{   
    Account temp;
    strcpy(temp.name, "unknown");
    strcpy(temp.password, "unknown");
    temp.status = 0;
	Node* head = makeNewNode(temp);

    readFileInput(&head);

    char choice[50];
    do{
        printf("\n\nUSER MANAGEMENT PROGRAM\n");
        printf("---------------------------------------------\n");
        printf("1. Register\n");
        printf("2. Activate\n");
        printf("3. Sign in\n");
        printf("4. Search\n");
        printf("5. Change password\n");
        printf("6. Sign out\n");
        printf("7. Homepage with domain name\n");
        printf("8. Homepage with IP address\n");
        printf("Your choice (1-8, other to quit): ");
        
        fgets(choice, sizeof(char)*50, stdin);
        trim(choice);
        if(strlen(choice)!=1 || choice[0]>'8' ||choice[0]<'1'){
            printf("\nEXIT\n");
            writeFile(head);
            return 1;
        }
        int choiceNumber = choice[0];

        switch (choiceNumber){
        case '1':
            registerAccount(&head);
            break;
        case '2':
            activateAccount(&head);
            break;
        case '3':
            signInAccount(&head);
            break;
        case '4':
            searchAccount(head);
            break;
        case '5':
            changePassword(&head);
            break;
        case '6':
            signOutAccount(head);
            break;
        case '7':
            HomepageWithDomainName(head);
            break;
        case '8':
            HomepageWithIPAddress(head);
            break;
        default:
            break;
        }
    }while (1);
	return 0;
}

//Linked List
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

//vào ra file
int readFileInput(Node** head){
    FILE *fin = fopen(filename, "r");
    FILE *ftest = fopen(filename, "r");
    if(fin==NULL || ftest==NULL){
        printf("ko mo duoc file input!\n");
        return 0;
    }
    char username[50], password[50], line[500];
    int status;
    char homepage[200];
    Account account;
    while (1)
    {
        memset(line, 0, strlen(line));
        fgets(line, sizeof(char)*500, ftest);
        if(strlen(line)<6){
            break;
        }
        fscanf(fin, "%s %s %d %s", username, password, &status, homepage);
        strcpy(account.name, username);
        strcpy(account.password, password);
        account.status = status;
        strcpy(account.homepage, homepage);
        addNode(head, account);
    }
    fclose(fin);
    fclose(ftest);

    FILE *flogin = fopen(filenameLogin, "r");
    if(flogin!=NULL){
        fscanf(flogin, "%s", usernameOfAccountLogin);
        fclose(flogin);
    }
}

int writeFile(Node* head){
    FILE *fout = fopen(filename, "w");
	Node* temp = (Node*) malloc(sizeof(Node));
	temp = head;
	while(temp!=NULL)
	{
		fprintf(fout, "%s %s %d %s\n", temp->account.name, temp->account.password, temp->account.status, temp->account.homepage);
		temp=temp->next;
	}
    fclose(fout);

    FILE *flogin = fopen(filenameLogin, "w");
    fprintf(flogin, "%s", usernameOfAccountLogin);
    fclose(flogin);
}

//xử lý input
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

//HOME PAGE
//tìm với domain name -> nếu tồn tại -> ok | 
//                    -> ko -> tìm với ip address
//  -> tồn tại -> ok
//  -> ko tồn tại -> not found infomation
struct hostent* getInfoByNameOrIP(char* arg){

    struct hostent* host = NULL;

    //tìm với domain name
    host = gethostbyname(arg);
    if(host!=NULL){
        struct in_addr addrName;
        addrName.s_addr = *(in_addr_t*)host->h_addr_list[0];

        struct in_addr test;
        inet_aton(host->h_name, &test);

        if(addrName.s_addr == test.s_addr){
            host = NULL;
        }

    }

    if(host!=NULL){
        return host;
    }

    //tìm với ip address
    struct in_addr addrIP;
    addrIP.s_addr = inet_addr(arg);
        
    if(addrIP.s_addr!=-1){
        host = gethostbyaddr(&addrIP, 4, AF_INET);
    }

    return host;
}

int printInfoName(struct hostent *host){

    printf("\nOfficial name: %s\n", host->h_name);

    for(int i=0; host->h_aliases[i]!=NULL; ++i){
        if(i==0){
            printf("Alias name:\n");
        }
        printf("%s\n", host->h_aliases[i]);
    }


}

int printInfoIP(struct hostent* host){

    struct in_addr addrName;
    addrName.s_addr = *(in_addr_t*)host->h_addr_list[0];
    printf("\nOfficial IP: %s\n", inet_ntoa(addrName));

    for(int i=1; host->h_addr_list[i]!=NULL; ++i){
        addrName.s_addr = *(in_addr_t*)host->h_addr_list[i];
        if(i==1){
            printf("Alias IP:\n");
        }
        printf("%s\n", inet_ntoa(addrName));
    }

}

//các chức năng
int registerAccount(Node **head){
    Account account;

    printf("Username: ");
    fgets(account.name, sizeof(char)*50, stdin);
    trim(account.name);

    if(strlen(account.name)==0 || xSpace(account.name) == -1){
        printf("\nUsername is invalid\n");
        return 0;
    }

    //check;
    if(getUser(*head, account.name)!=NULL){
        printf("\nAccount existed\n");
        return 0;
    }

    printf("Password: ");
    fgets(account.password, sizeof(char)*50, stdin);
    trim(account.password);

    if(strlen(account.password)==0 || xSpace(account.password) == -1){
        printf("\nPassword is invalid\n");
        return 0;
    }

    printf("Homepage: ");
    fgets(account.homepage, sizeof(char)*200, stdin);
    trim(account.homepage);

    if(strlen(account.homepage)==0 || xSpace(account.homepage) == -1){
        printf("\nHomepage is invalid\n");
        return 0;
    }

    account.status = STATUS_IDLE;

    printf("\nSuccessful registration. Activation required.\n");
    addNode(head, account);
    writeFile(*head);
}

int activateAccount(Node **head){
    Account account;
    Node* user;

    printf("Username: ");
    fgets(account.name, sizeof(char)*50, stdin);
    trim(account.name);

    //check;
    if((user=getUser(*head, account.name))==NULL){
        printf("\nCannot find account\n");
        return -1;
    }

    printf("Password: ");
    fgets(account.password, sizeof(char)*50, stdin);
    trim(account.password);

    //check
    if(strcmp(user->account.password, account.password)!=0){
        printf("\nPassword is incorrect\n");
        return -1;
    }

    if(user->account.status == STATUS_ACTIVE){
        printf("\nAccount is activated\n");
        return 1;
    }else if(user->account.status == STATUS_BLOCKED){
        printf("\nAccount is blocked\n");
        return 1;
    }

    printf("Code: ");
    char code[50];
    fgets(code, sizeof(code)*50, stdin);
    trim(code);

    //check
    int codeNumber = atoi(code);
    if(codeNumber == ACTIVATION_CODE){
        printf("\nAccount is activated\n");
        setStatus(head, account.name, STATUS_ACTIVE);
        writeFile(*head);
        return 1;
    }
    
    int check = setCheckNode(head, account.name);
    if(check<5){
        printf("\nAccount is not activated");
    }else{
        printf("\nActivation code is incorrect.\n");
        printf("Account is blocked\n");
        setStatus(head, account.name, STATUS_BLOCKED);
        writeFile(*head);
    }

}

int signInAccount(Node **head){

    if(strlen(usernameOfAccountLogin)!=0){
        printf("\nPlease log out of the account logged in!\n");
        return -1;
    }

    Account account;
    Node* user;

    printf("Username: ");
    fgets(account.name, sizeof(char)*50, stdin);
    trim(account.name);

    //check;
    if((user=getUser(*head, account.name))==NULL){
        printf("\nCannot find account\n");
        return -1;
    }

    printf("Password: ");
    fgets(account.password, sizeof(char)*50, stdin);
    trim(account.password);

    //check
    if(strcmp(user->account.password, account.password)!=0){
        if(setCheckNode(head, account.name)<4){
            printf("\nPassword is incorrect\n");
        }else{
            setStatus(head, account.name, STATUS_BLOCKED);
            printf("\nPassword is incorrect\n");
            printf("Account is blocked\n");
        }
        return -1;
    }

    if(user->account.status == STATUS_ACTIVE){
        printf("\nHello %s\n", user->account.name);
        strcpy(usernameOfAccountLogin, user->account.name);
        writeFile(*head);
    }else if(user->account.status == STATUS_BLOCKED){
        printf("\nAccount is blocked\n");
    }else if(user->account.status == STATUS_IDLE){
        printf("\nAccount is idle\n");
    }
    return 1;
}

int searchAccount(Node *head){
    Account account;
    Node* user;

    printf("Username: ");
    fgets(account.name, sizeof(char)*50, stdin);
    trim(account.name);

    //check;
    if(strlen(usernameOfAccountLogin)==0){
        printf("\nAccount is not sign in\n");
        return -1;
    }

    if((user=getUser(head, account.name))==NULL){
        printf("\nCannot find account\n");
        return -1;
    }

    if(user->account.status == STATUS_ACTIVE){
        printf("\nAccount is active\n");
        strcpy(usernameOfAccountLogin, user->account.name);
    }else if(user->account.status == STATUS_BLOCKED){
        printf("\nAccount is blocked\n");
    }else if(user->account.status == STATUS_IDLE){
        printf("\nAccount is idle\n");
    }
    return 1;
    
}

int signOutAccount(Node* head){
    Account account;
    Node* user;

    printf("Username: ");
    fgets(account.name, sizeof(char)*50, stdin);
    trim(account.name);

    //check;
    if((user=getUser(head, account.name))==NULL){
        printf("\nCannot find account\n");
        return -1;
    }

    if(strcmp(account.name, usernameOfAccountLogin)!=0){
        printf("\nAccount is not sign in\n");
        return -1;
    }

    printf("\nGoodbye %s\n", account.name);
    memset(usernameOfAccountLogin, 0, strlen(usernameOfAccountLogin));
    writeFile(head);
    return 1;
}

int changePassword(Node **head){
    Account account;
    Node* user;

    printf("Username: ");
    fgets(account.name, sizeof(char)*50, stdin);
    trim(account.name);

    //check;
    if((user=getUser(*head, account.name))==NULL){
        printf("\nCannot find account\n");
        return -1;
    }

    if(strcmp(account.name, usernameOfAccountLogin)!=0){
        printf("\nAccount is not sign in\n");
        return -1;
    }

    printf("Password: ");
    fgets(account.password, sizeof(char)*50, stdin);
    trim(account.password);

    char newPassword[50];
    printf("New Password: ");
    fgets(newPassword, sizeof(char)*50, stdin);
    trim(newPassword);

    if(strcmp(user->account.password, account.password)!=0){
        printf("\nCurrent password is incorrect. Please try again\n");
        return -1;
    }

    if(xSpace(newPassword) == -1){
        printf("\nNew Password is invalid\n");
        return -1;
    }

    printf("\npassword is change\n");
    setPassword(head, account.name, newPassword);
    writeFile(*head);
}

int HomepageWithDomainName(Node* head){
    if(strlen(usernameOfAccountLogin)==0){
        printf("\nAccount is not sign in\n");
        return -1;
    }
    
    Node* user = getUser(head, usernameOfAccountLogin);

    struct hostent* host = getInfoByNameOrIP(user->account.homepage);

    if(host==NULL){
        printf("\nNot found infomation!\n");
        return 0;
    }
    
    printInfoName(host);

}

int HomepageWithIPAddress(Node* head){
    if(strlen(usernameOfAccountLogin)==0){
        printf("\nAccount is not sign in\n");
        return -1;
    }

    Node* user = getUser(head, usernameOfAccountLogin);

    struct hostent* host = getInfoByNameOrIP(user->account.homepage);

    if(host==NULL){
        printf("\nNot found infomation!\n");
        return 0;
    }

    printInfoIP(host);

}
