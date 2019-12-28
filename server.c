#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define UNKNOWN "UNKNOWN"

#define BACKLOG 30
#define BUFF_SIZE 8192
#define SINIT "SINIT"
#define ALERT "ALERT"
#define LOGIN "LOGIN"
#define UPASS "UPASS"
#define LOGOU "LOGOU"
#define SIGNU "SIGNU"
#define SIGNP "SIGNP"
#define SIGNC "SIGNC"

// Message tra ve client 
#define C_CHANGED_STATUS "99"
#define C_INIT_SUCESS "100"

// Signup
// SS: WAIT_FOR_USERNAME_SIGNUP 11
#define C_SAME_USER "110"
#define C_NEW_USER "111"
#define C_MAX_USER "112"
// SS: WAIT_FOR_PASS 12
#define C_CORRECT_PASS "120"
#define C_INCORRECT_PASS "121"



#define C_FOUND_ID "00"
#define C_NOT_FOUND_ID "01"
#define C_LOGOUT_OK "20"
#define C_LOGOUT_FAILS "21"
#define C_BLOCK "31"

#define C_FOUND_PASSWORD "10"
#define C_NOT_FOUND_PASSWORD "11"
#define C_CORRECT_CODE "60"
#define C_INCORRECT_CODE "61"

// Status cua server
#define WAIT_FOR_REQUEST 0
#define WAIT_FOR_USERNAME_SIGNUP 11
#define WAIT_FOR_PASS 12

#define NOT_IDENTIFIED_USER 1
#define NOT_AUTHENTICATED 2
#define AUTHENTICATED 3
#define START_SIGNUP 4

#define PASSWORD_CREATED 6
#define SIGNUP_SUCCESSFUL 7

// Status isLogin cua session
#define NOT_LOGIN 0
#define LOGIN 1

#define BLOCKED 0
#define ACTIVE 1
#define WAIT 0
#define PLAY 1
#define MAX_NUMBER_LOGIN 10
#define MAX_USER 10
#define MAX_ROOM 10
// To do diff
#define MAX_SESSION 100
#define FILE_NAME "account.txt"

//To do diff

struct User
{
	char id[30];
	char password[30];
	int userStatus;
	int count;
};

struct User users[MAX_USER];
struct User userSigns[MAX_USER];

struct Session
{
	struct User user;
	int sessStatus;
	int isLogin;
	int countLogin;
	int connd;
	char capcha[6];
	struct sockaddr_in cliAddr;
	// struct Room room; //to do diff
};

struct Session sess[MAX_SESSION];

struct Session sessSignup[MAX_SESSION];

int sessCount = 0;
int userCount = 0;
int sessSignCount = 0;
int posCapchar;

//user constructor
struct User newUser(char id[], char password[], int userStatus)
{
	struct User user;
	strcpy(user.id, id);
	strcpy(user.password, password);
	user.userStatus = userStatus;
	user.count = 0;		//STT cua user va dem so user luon, vi tri cua user
	return user;
}

//session constructor
struct Session newSession(struct User user, int sessStatus, struct sockaddr_in cliAddr, int connd)
{
	struct Session session;
	session.user = user;
	session.sessStatus = sessStatus;
	session.isLogin = NOT_LOGIN;
	session.cliAddr = cliAddr;
	session.countLogin = 0;
	session.connd = connd;
	return session;
}

// To do diff

void printSession(int pos)
{
	if (pos <= 0) return;
	printf("Session status: %d\n", sess[pos].sessStatus);
	printf("Session countlogin: %d\n", sess[pos].countLogin);
	// to do diff -> printRoom
}

void printStatus(char when[],int pos){
	if (pos < 0) return;
	printf("%s: \n",when);
	printf("\tSession pos: %d\n", pos);
	printf("\tSession status: %d\n", sess[pos].sessStatus);
	printf("\tSession user: %s\n", sess[pos].user.id);
}

// To do diff

//check message from client is valid? //need refine
int isValidMessage(char message[], char messCode[], char messAcgument[])
{
	int i = 0, j = 0;
	for (i = 0; i < 5; i++ ){
		messCode[i] = message[i];
	}
	messCode[i] = '\0';
	if (message[i] != ' ') return 0;
	while (message[++i] != '\n'){
		messAcgument[j++] = message[i];
		if (message[i] == ' ') return 0;
	}
	messAcgument[j] = '\0';
	// printf("|%s|_|%s|\n",messCode,messAcgument);
	return 1;
}

int receive(int conn_sock, char message[])
{
	int bytes_received = recv(conn_sock, message, BUFF_SIZE -1, 0);
	if (bytes_received > 0)
	{
		message[bytes_received] = '\0';
		return 1;
	} else return 0;
}

int respond(int conn_sock, char respond[])
{
	if (send(conn_sock, respond, strlen(respond), 0) > 0)
	{
		return 1;
	} else return 0;
}

//read file and save to struct User
void readFileUser(char filename[])
{
	FILE *f = fopen(filename,"r");
	userCount = 0;
	int i = 0;
	char id[30], password[30], userStatus[2];
	struct User user;
	if (f == NULL)
	{
		printf("Can't open file %s!\n", filename);
		return; 
	}else {
		while (!feof(f))
		{
			fscanf(f, "%s %s %s\n", id, password, userStatus);
			user = newUser(id, password, atoi(userStatus));
			users[i] = user;
			users[i].count = i + 1;		//luu STT
			userCount++;
			i++;
		}
	}
	fclose(f);
}

//show user detail
void showUser()
{
	int i;
	printf("List user information: \n");
	for (i = 0; users[i].count <= MAX_USER && users[i].count>0; i++ )
	{
		printf("----------------------------------------------\n");
		printf("Id : %s\n",users[i].id);
		printf("Password : %s\n",users[i].password);
		printf("Status : %d\n",users[i].userStatus);
		printf("No: %d\n",users[i].count);
	}
	printf("-----------------------------------------------------\n");
}

//write user to file
void writeUserToFile(char filename[])
{
	FILE *f = fopen(filename,"w+");
	int i = 0;
	struct User user;
	if (f == NULL)
	{
		printf("Can't open file %s!\n",filename);
		return; 
	}else {
		showUser();
		for (i = 0; users[i].count <= MAX_USER && users[i].count>0; i++ )
		{
			fprintf(f, "%s %s %d\n", users[i].id, users[i].password, users[i].userStatus);
		}
	}
	fclose(f);
}


//find user by userID
int findUserById(char messAcgument[])
{
	int i = 0;
	for (i = 0; users[i].count != 0; i++ ){
		if (strcmp(users[i].id, messAcgument) == 0)
		{
			return i;
		} 
	}
	return -1;
}

//add a new session
int addUser(struct User user)
{
	if (userCount > MAX_USER) return 0;
	//TRY: Thu MAX_USER>10
	users[userCount++] = user;
	return 1;
}

//add a new session  // to do: check sess max
int addSession(struct Session session)
{
	sess[sessCount++] = session;
}

//add a new session signup // to do: check sess max
int addSessionSignup(struct Session session)
{
	sessSignup[sessSignCount++] = session;
}

//remove session  // need refine 
int removeSession(int k)
{
	int i;
	for (i = k; i < sessCount -1; ++i)
	{
		sess[k] = sess[k+1];
	}
}

//find session by cliAddr, return session position
int findSessByAddr(struct sockaddr_in cliAddr, int connd)
{
	int i = 0;
	for (i = 0; i < sessCount; i++ ){
	 	if (memcmp(&(sess[i].cliAddr), &cliAddr, sizeof(struct sockaddr_in)) == 0 && sess[i].connd == connd)
		{
			return i;
		}
	}
	return -1;
}

//todo diff

int checkPass(char pass[]){
	//Password has min_length is 5
	int i;
	printf("pass: %s_%d\n",pass,strlen(pass));
	if (strlen(pass) < 5) return 0;
	else return 1;
}

//create capcha code, include 6 random character
char *makeCapcha(){
	char *capcha = (char*) malloc(6*sizeof(char));
	int i;
	srand(time(NULL));
	for(i = 0; i < 6; ++i){
    		capcha[i] = '0' + rand()%72; // starting on '0', ending on '}'
	}
	capcha[6]='\0';
	return capcha;
}

//find sessionSignup by cliAddr, return session position
int findSessSignByAddr(struct sockaddr_in cliAddr, int connd)
{
	int i = 0;
	for (i = 0; i < sessSignCount; i++ ){
	 	if (memcmp(&(sessSignup[i].cliAddr), &cliAddr, sizeof(struct sockaddr_in)) == 0 && sessSignup[i].connd == connd)
		{
			return i;
		}
	}
	return -1;
}

char *initSession(struct sockaddr_in cliAddr, int connd){
	int pos;
	struct Session session;
	struct User user;
	
	pos=findSessByAddr(cliAddr,connd);
	if (pos==-1){
		user = newUser(UNKNOWN, "", ACTIVE);
		session = newSession(user, WAIT_FOR_REQUEST, cliAddr, connd);// create new session
		addSession(session);
		pos=findSessByAddr(cliAddr,connd);
		printStatus("SINIT",pos);
		return C_INIT_SUCESS;
	}
	printf("==>Init error!\n");
	return "==>Init error!\n";
}

//to do diff

//process while Code is ALERT
char *alertCodeProcess(char messAcgument[],struct sockaddr_in cliAddr, int connd, int pos){
	//found session
	if (atoi(messAcgument)==WAIT_FOR_USERNAME_SIGNUP && userCount == MAX_USER)
	{
		return C_MAX_USER;
	}
	sess[pos].sessStatus = atoi(messAcgument); //WAIT_FOR_USERNAME_SIGNUP
	printStatus("ALERT",pos);
	return C_CHANGED_STATUS;
}

//process while Code is LOGIN
char *userCodeProcess(struct sockaddr_in cliAddr, int connd, int pos, int i)
{
	struct Session session;
	if (i == -1) return C_NOT_FOUND_ID; //if not found user
	if (users[i].userStatus == BLOCKED)	return C_BLOCK; //if found user but user blocked
	if (pos == -1) //if not found session
	{
		session = newSession(users[i], NOT_AUTHENTICATED, cliAddr, connd);// create new session
		addSession(session);
		printf("session status:%d\n",sess[sessCount-1].sessStatus);                                       // add session
		return C_FOUND_ID;
	}
	//found session
	else if (sess[pos].sessStatus == NOT_AUTHENTICATED || sess[pos].sessStatus == NOT_IDENTIFIED_USER) {//found user != user of session
		sess[pos].sessStatus = NOT_AUTHENTICATED;
		// printf("aaa\n");
		printf("session status:%d\n",sess[pos].sessStatus);
		memcpy(&(sess[pos].user), &users[i], sizeof(struct User));
		return C_FOUND_ID;
	}
	return "Login Sequence Is Wrong";
}

//Process while Code is UPASS
char *passCodeProcess(char messAcgument[], int pos)
{
	int i;
	printf("session status:%d\n",sess[pos].sessStatus);
	if (sess[pos].user.userStatus == BLOCKED)
		{
			return C_BLOCK;
		}
		//if PASS ok
		if (strcmp (sess[pos].user.password, messAcgument) == 0) 
		{
			sess[pos].sessStatus = AUTHENTICATED; //next status  
			sess[pos].countLogin = 0;			 // reset count login
			return C_FOUND_PASSWORD;
		}
		//PASS error 
		else 
		{                                                
			sess[pos].countLogin++;    			//countLogin + 1
			if (sess[pos].countLogin >= MAX_NUMBER_LOGIN){  //check count login is > max?
				sess[pos].user.userStatus = BLOCKED;	//block user 
				i = findUserById (sess[pos].user.id);
				users[i].userStatus = BLOCKED;
				writeUserToFile(FILE_NAME);			//save to file
				return C_BLOCK;
			} else
				return C_NOT_FOUND_PASSWORD;
		}
}

//to do diff

//Process while Code is LOGOU
char *loutCodeProcess(char messAcgument[], int pos)
{
	if (strcmp(sess[pos].user.id, messAcgument) == 0) //check userId is valid?
		{
			sess[pos].sessStatus = NOT_IDENTIFIED_USER; //reset session status
			return C_LOGOUT_OK;
		} else 
			return C_LOGOUT_FAILS;
}

//process while code is SIGNU
char *siguCodeProcess(char messAcgument[],struct sockaddr_in cliAddr, int connd, int pos, int i)
{
    struct Session session;
	struct User user;
	if (userCount == MAX_USER) return C_MAX_USER; //If max user
	if (i != -1) return C_SAME_USER; //if not found user
	user = newUser(messAcgument, "", ACTIVE);
	printf("%d)\n",sess[pos].sessStatus);
	if (sess[pos].sessStatus == WAIT_FOR_USERNAME_SIGNUP) {//found user != user of session
		sess[pos].sessStatus = WAIT_FOR_PASS;
		memcpy(&(sess[pos].user), &user, sizeof(struct User));
		printStatus("SIGNU",pos);
		return C_NEW_USER;
	}
	return "Sequence Is Wrong";
}


//Process while Code is SIGNP
char *sigpCodeProcess(char messAcgument[], int pos)
{
	struct User user;
	// printf("Vao 11\n");
	if (checkPass(messAcgument)){

		strcpy(sess[pos].user.password, messAcgument);
		sess[pos].user.count = userCount+1;

		// NOTE: Da chan truong hop max_user o signup process
		if (addUser(sess[pos].user))
		{
			writeUserToFile(FILE_NAME);
			sess[pos].sessStatus = WAIT_FOR_REQUEST;  //SIGNUP SUCESS //back to default status 
			user = newUser(UNKNOWN, "", ACTIVE); //reset sess.user
			memcpy(&(sess[pos].user), &user, sizeof(struct User));
			printStatus("SIGNP",pos);
			return C_CORRECT_PASS;
		}
	}
	else return C_INCORRECT_PASS;
	return "Sequence Is Wrong";
}

//Process while Code is SIGNC
char *sigcCodeProcess(char messAcgument[], int pos)
{
	if (strcmp(sessSignup[pos].capcha, messAcgument) == 0) //check capcha
	{
		sessSignup[pos].sessStatus = SIGNUP_SUCCESSFUL; 
		sessSignup[pos].user.count = userCount;
		if (addUser(sessSignup[pos].user))
		{
			writeUserToFile(FILE_NAME);
			return C_CORRECT_CODE;
		}
	} else 
		return C_INCORRECT_CODE;
}

//process request
char *process(char messCode[], char messAcgument[], struct sockaddr_in cliAddr, int connd )
{
	int pos,  posSign, i;

	pos = findSessByAddr(cliAddr, connd); //find Session return -1 if session not exists
	posSign = findSessSignByAddr(cliAddr, connd);

	// //test
	printf("\tSession pos: %d\n",pos);
	printf("\tSession status: %d\n",sess[pos].sessStatus);

	// checkListRoom(); //todo diff

	/***********messcode is SINIT***********/ 
	// Khoi tao status ben server
	if (strcmp(messCode, SINIT) == 0 ){
		return initSession(cliAddr,connd);
	}
	
	/***********messcode is LOGIN***********/
	if (strcmp(messCode, ALERT) == 0 ){
		return alertCodeProcess(messAcgument,cliAddr,connd, pos);
	}
	/***********messcode is LOGIN***********/
	if (strcmp(messCode, LOGIN) == 0 ){
		// printSession(pos);
		i = findUserById (messAcgument); //find user return -1 if user not exists
		return userCodeProcess(cliAddr,connd, pos, i);
	}

	/********messcode is UPASS**********/
	if (strcmp(messCode, UPASS) == 0 && pos != -1 && sess[pos].sessStatus == NOT_AUTHENTICATED )
	{
		// printSession(pos);
		printf("session status:%d\n",sess[pos].sessStatus);
		return passCodeProcess(messAcgument, pos);
	}
    
    /********messcode is SIGNU*********/
	if (strcmp(messCode, SIGNU) == 0)
	{
        i = findUserById (messAcgument); //find user return -1 if user not exists
		return siguCodeProcess(messAcgument, cliAddr,connd, pos, i);
	}

	/********messcode is SIGNP*********/
	if (strcmp(messCode, SIGNP) == 0 && sess[pos].sessStatus == WAIT_FOR_PASS)
	{
		return sigpCodeProcess(messAcgument, pos);
	}

	/********messcode is SIGNC*********/
	if (strcmp(messCode, SIGNC) == 0 && posSign != -1 && sessSignup[posSign].sessStatus == PASSWORD_CREATED)
	{
		return sigcCodeProcess(messAcgument, posSign);
	}
    
    /********messcode is LOGOU*********/
	if (strcmp(messCode, LOGOU) == 0 && pos != -1 && sess[pos].sessStatus == AUTHENTICATED)
	{
		return loutCodeProcess(messAcgument, pos);
	}
	else
	{
		return "Login Sequence Is Wrong";
	}
}

//convert to full message
void changeFull(char message[])
{
	if (strcmp(message, C_FOUND_PASSWORD) == 0)
	{
		strcat(message, " -> Password ok. Login successful!\n");
		// to do diff. for more info
	}
	// if (strcmp(message, C_CORRECT_PASS) == 0)
	// {
	// 	char capcha[6];
	// 	strcpy(capcha, sessSignup[posCapchar].capcha);
	// 	printf("cap:%s\n",capcha);
	// 	strcat(message, " -> Please enter capcha code : ");
	// 	strcat(message, capcha);
	// }
}

int main(int argc, char *argv[]) {
	if (argc != 2) return -1;
 	int PORT = atoi(argv[1]);
	char buff[BUFF_SIZE];
	char message[BUFF_SIZE], messCode[BUFF_SIZE], messAcgument[BUFF_SIZE];
	struct pollfd fds[BACKLOG];
	struct sockaddr_in server_addr, client_addr;
	int sin_size = sizeof(client_addr);
	int listen_sock, fdmax, newfd,nbytes,i;

	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) <0) {
		perror("Error socket()");
		exit(1);
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);
	if (bind(listen_sock, (struct sockaddr *) &server_addr, sizeof(server_addr))<0) {
		perror("Error bind()");
		exit(1);
	}

	if (listen(listen_sock, BACKLOG) == -1) {
		perror("Error listen()");
		exit(1);
	}
	printf("...\n");

    // to do sum server response
	// readQues();
	// printListQues();
	fdmax = 1;
	fds[0].fd = listen_sock;
	fds[0].events = POLLIN;
	while(1) {
		if (poll(fds, fdmax, -1) == -1) {
			perror("Error poll()");
			exit(1);
		}
		for (i = 0; i < fdmax; i++) {
			if (fds[i].revents != 0) {
				if (fds[i].fd == listen_sock) {
					if ((newfd = accept(listen_sock, (struct sockaddr *) &client_addr, (socklen_t*) &sin_size)) < 0) {
						perror("Error accept()");
					} 
					else {
						if (newfd > fdmax) {
							fdmax = newfd;
						}
						printf("Connected\n");
						fds[newfd].fd = newfd;
						fds[newfd].events = POLLIN;
						fds[newfd].revents = 0;
						fdmax++;
						printf("You got a connection from %s\n", inet_ntoa(client_addr.sin_addr));
						// initSession(client_addr,fds[i].fd);
						// printf("%d_done\n",fds[i].fd);
					}
				} 
				else if(fds[i].revents & POLLIN){
					readFileUser (FILE_NAME);
					// showUser();
					//recieve data
					if ((nbytes = recv(fds[i].fd, buff,BUFF_SIZE, 0)) <= 0) {
						if (nbytes == 0)
							printf("Server: socket %d out\n", fds[i].fd);
						close(fds[i].fd);
					}

					else {
						buff[nbytes]='\0';

						if (isValidMessage (buff, messCode, messAcgument))
						{
							printf("-----------------------------------------------------------\n");
							printf("RUNNING\n\tmessCode:%s\n\tmessAcgument:%s\n",messCode, messAcgument );

							strcpy(message, process(messCode, messAcgument, client_addr, fds[i].fd));

							printf("\t\t==>Return to client: %s\n",message);
							changeFull(message);
						}else{
							strcpy(message, "Syntax Error!");
						}
						//send data
						if (strcmp(message, "NULL") != 0)
						{
							respond(fds[i].fd,message);
							bzero(buff,BUFF_SIZE);
						}else{
							printf("NULL\n");
						}
					}
				}
			}
		}
	}
	return 0;
}