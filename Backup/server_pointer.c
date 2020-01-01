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
#define RESET "RESET"
#define LOGIN "LOGIN"
#define UPASS "UPASS"
#define LOGOU "LOGOU"
#define SIGNU "SIGNU"
#define SIGNP "SIGNP"
#define SIGNC "SIGNC"
#define CROOM "CROOM"
#define SROOM "SROOM"
#define UJOIN "UJOIN"

// Message tra ve client
#define C_CHANGED_STATUS "99"
#define C_INIT_SUCESS "100"

// Notification
#define C_NO_INFO "404"
#define C_USER_JOIN "714"

// Signup
// SS: WAIT_FOR_USERNAME_SIGNUP 11
#define C_SAME_USER "110"
#define C_NEW_USER "111"
#define C_MAX_USER "112"
// SS: WAIT_FOR_PASS_SIGNUP 12
#define C_CORRECT_PASS "120"
#define C_INCORRECT_PASS "121"

// Login Return
// SS: WAIT_FOR_USERNAME_LOGIN 21
#define C_FOUND_ID "210"
#define C_NOT_FOUND_ID "211"
#define C_IS_LOGIN "212"
// SS: WAIT_FOR_PASS_LOGIN 22
#define C_FOUND_PASSWORD "220"
#define C_NOT_FOUND_PASSWORD "221"

// Logout Return
// SS: WAIT_FOR_USERNAME_LOGOUT 41
#define C_LOGOUT_OK "410"
#define C_LOGOUT_FAILS "411"

// Creat Room Return
// SS: WAIT_FOR_RNAME_CREATE_ROOM 51
#define C_CRE_ROOM_SUC "510"
#define C_CRE_ROOM_FAI "511"

// See Room Return
// SS: WAIT_FOR_SEE_ROOM 61
#define C_SEE_ROOM_RESULT "610"

// Join Room Return
// SS: WAIT_FOR_RNAME_JOIN_ROOM 71
#define C_IN_ROOM "710"
#define C_NON_EXIST_ROOM "711"
#define C_PLAYING_ROOM "712"
#define C_MAX_USER_IN_ROOM "713"

// Unuse
#define C_BLOCK "31"
#define C_CORRECT_CODE "60"
#define C_INCORRECT_CODE "61"
#define PASSWORD_CREATED 6
#define SIGNUP_SUCCESSFUL 7
#define NOT_IDENTIFIED_USER 1
#define NOT_AUTHENTICATED 2
#define AUTHENTICATED 3
#define START_SIGNUP 4

// Status cua client
#define IN_MENU "0"
#define IN_MENU_LOGGED "3"

// Status cua server
#define WAIT_FOR_REQUEST 0
#define WAIT_FOR_USERNAME_SIGNUP 11
#define WAIT_FOR_PASS_SIGNUP 12
#define WAIT_FOR_USERNAME_LOGIN 21
#define WAIT_FOR_PASS_LOGIN 22
#define WAIT_FOR_USERNAME_LOGOUT 41
#define WAIT_FOR_RNAME_CREATE_ROOM 51
#define WAIT_FOR_SEE_ROOM 61
#define WAIT_FOR_RNAME_JOIN_ROOM 71

// Status isLogin cua session
#define USER_NOT_LOGIN 0
#define USER_LOGIN 1

#define BLOCKED 0
#define ACTIVE 1
#define WAIT 0
#define PLAY 1

#define MAX_NUMBER_LOGIN 10
#define MAX_USER 10
#define MAX_USER_IN_ROOM 2
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
	char message[BUFF_SIZE]; //message send to user for notification
};

struct User users[MAX_USER];
struct User user_temp; //for signup

// NGUYEN: Dinh nghia kieu du lieu game
// typedef struct question
// {
// 	int id;
// 	int level; //1 : easy, 2 : medium, 3 : hard
// 	char content[100];
// 	char choiceA[50];
// 	char choiceB[50];
// 	char choiceC[50];
// 	char choiceD[50];
// 	char answer;
// 	int quesStatus; //da co nguoi tra loi nhanh nhat chua?
// } Question;

struct Room
{
	int id;
	char name[BUFF_SIZE];
	struct User *users[MAX_USER_IN_ROOM];
	int connd[MAX_USER_IN_ROOM];
	int countUser;
	// NGUYEN: khai bao kieu du lieu game o day
	// Question questions[MAX_QUESTION];
	// int countQues;
	int roomStatus;
};

struct Room rooms[MAX_ROOM];
int countRoom = 0;

struct Session
{
	struct User *user;
	int sessStatus;
	int isLogin;
	int countLogin;
	int connd;
	char capcha[6]; //REFRACTOR: Bo capcha
	char temp[BUFF_SIZE]; //Just save sth tam thoi
	struct sockaddr_in cliAddr;
	struct Room *room;
};
struct Session sess[MAX_SESSION];

struct Session sessSignup[MAX_SESSION]; //REFRACTOR: Bo sessSIgnup

int sessCount = 0;
int userCount = 0;
int sessSignCount = 0; //REFRACTOR: Bo capcha
int posCapchar;		   //REFRACTOR: Bo capcha

//user constructor
struct User newUser(char id[], char password[], int userStatus)
{
	struct User user;
	strcpy(user.id, id);
	strcpy(user.password, password);
	user.userStatus = userStatus;
	strcpy(user.message,C_NO_INFO);
	user.count = 0; //STT cua user va dem so user luon, vi tri cua user
	return user;
}

//session constructor
struct Session newSession(struct User user, int sessStatus, struct sockaddr_in cliAddr, int connd)
{
	struct Session session;
	session.user = &user;
	session.sessStatus = sessStatus;
	session.isLogin = USER_NOT_LOGIN;
	session.cliAddr = cliAddr;
	session.countLogin = 0;
	session.connd = connd;
	session.room = NULL;
	return session;
}

//room constructor
struct Room newRoom(char room_name[], int connd, struct User user)
{
	int i;
	struct Room room;
	// REFRACTOR
	for (i = 0; i < countRoom; ++i)
	{
		if (strcmp(rooms[i].name, room_name) == 0)
		{
			return rooms[i];
		}
	}
	rooms[countRoom].id = countRoom + 1;
	strcpy(rooms[countRoom].name, room_name);
	rooms[countRoom].users[0] = &user;
	rooms[countRoom].countUser = 1;
	rooms[countRoom].connd[0] = connd;
	// NGUYEN:
	// rooms[countRoom].countQues = 0;
	rooms[countRoom].roomStatus = WAIT; //DOTO
	countRoom++;
	return rooms[countRoom - 1];
}

// to do diff

//
void printRoom(struct Room room)
{
	printf("\tRoom id :%d\n", room.id);
	printf("\tRoom name :%s\n", room.name);
	printf("\tRoom countUser :%d\n", room.countUser);
	// NGUYEN
	// printf("room countQues :%d\n",room.countQues );
	printf("\tRoom status :%d\n", room.roomStatus);
}

//REFRACTOR
void printSession(int pos)
{
	if (pos <= 0)
		return;
	printf("Session status: %d\n", sess[pos].sessStatus);
	printf("Session countlogin: %d\n", sess[pos].countLogin);
	// to do diff -> printRoom
}

void printStatus(char when[], int pos)
{
	if (pos < 0)
		return;
	printf("%s: \n", when);
	printf("\tSession pos: %d\n", pos);
	printf("\tSession status: %d\n", sess[pos].sessStatus);
	printf("\tSession is login: %d\n", sess[pos].isLogin);
	printf("\tSession user: %s\n", sess[pos].user->id);
	printf("\tSession cond: %d\n", sess[pos].connd);

	if (sess[pos].room != NULL)
		printRoom(*sess[pos].room);
}

// To do diff

//check message from client is valid? //need refine
int isValidMessage(char message[], char messCode[], char messAcgument[])
{
	int i = 0, j = 0;
	for (i = 0; i < 5; i++)
	{
		messCode[i] = message[i];
	}
	messCode[i] = '\0';
	if (message[i] != ' ')
		return 0;
	while (message[++i] != '\n')
	{
		messAcgument[j++] = message[i];
		if (message[i] == ' ')
			return 0;
	}
	messAcgument[j] = '\0';
	return 1;
}

int isLogin(char messAcgument[]){
	int i = 0;
	for (i = 0; i < sessCount; i++)
	{
		if (strcmp(sess[i].user->id,messAcgument) == 0){
			return i;
		}
	}
	return -1; //chua login
}

int receive(int conn_sock, char message[])
{
	int bytes_received = recv(conn_sock, message, BUFF_SIZE - 1, 0);
	if (bytes_received > 0)
	{
		message[bytes_received] = '\0';
		return 1;
	}
	else
		return 0;
}

int respond(int conn_sock, char respond[])
{
	if (send(conn_sock, respond, strlen(respond), 0) > 0)
	{
		return 1;
	}
	else
		return 0;
}

//read file and save to struct User
void readFileUser(char filename[])
{
	FILE *f = fopen(filename, "r");
	userCount = 0;
	int i = 0;
	char id[30], password[30], userStatus[2];
	struct User user;
	if (f == NULL)
	{
		printf("Can't open file %s!\n", filename);
		return;
	}
	else
	{
		while (!feof(f))
		{
			fscanf(f, "%s %s %s\n", id, password, userStatus);
			user = newUser(id, password, atoi(userStatus));
			users[i] = user;
			users[i].count = i + 1; //luu STT
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
	for (i = 0; users[i].count <= MAX_USER && users[i].count > 0; i++)
	{
		printf("----------------------------------------------\n");
		printf("Id : %s\n", users[i].id);
		printf("Password : %s\n", users[i].password);
		printf("Status : %d\n", users[i].userStatus);
		printf("No: %d\n", users[i].count);
	}
	printf("-----------------------------------------------------\n");
}

//write user to file
void writeUserToFile(char filename[])
{
	FILE *f = fopen(filename, "w+");
	int i = 0;
	struct User user;
	if (f == NULL)
	{
		printf("Can't open file %s!\n", filename);
		return;
	}
	else
	{
		showUser();
		for (i = 0; users[i].count <= MAX_USER && users[i].count > 0; i++)
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
	for (i = 0; users[i].count != 0; i++)
	{
		if (strcmp(users[i].id, messAcgument) == 0)
		{
			return i;
		}
	}
	return -1;
}

//find user in sess.room (room in session) - return pos of user in room structure
int findUserInSessRoom(char messAcgument[], int pos)
{
	int i = 0;
	for (i = 0; i<MAX_USER_IN_ROOM; i++)
	{
		if (strcmp(sess[pos].room->users[i]->id, messAcgument) == 0)
		{
			return i;
		}
	}
	return -1;
}

//add a new session
int addUser(struct User user)
{
	if (userCount > MAX_USER)
		return 0;
	// TRY: Thu MAX_USER>10
	users[userCount++] = user;
	return 1;
}

//add a new session  // DOTO: check sess max
int addSession(struct Session session)
{
	sess[sessCount++] = session;
}

//add a new session signup // REFRACTOR
int addSessionSignup(struct Session session)
{
	sessSignup[sessSignCount++] = session;
}

//remove session  // REFRACTOR
int removeSession(int k)
{
	int i;
	for (i = k; i < sessCount - 1; ++i)
	{
		sess[k] = sess[k + 1];
	}
}

//find session by cliAddr, return session position
int findSessByAddr(struct sockaddr_in cliAddr, int connd)
{
	int i = 0;
	for (i = 0; i < sessCount; i++)
	{
		// printf("\t_Session pos: %d\n", connd);
		// printf("\t_You got a connection from %s\n", inet_ntoa(cliAddr.sin_addr));
		// printf("\t__Session pos: %d\n", sess[i].connd);
		// printf("\t__You got a connection from %s\n",  inet_ntoa(sess[i].cliAddr.sin_addr));

		// if (memcmp(&(sess[i].cliAddr), &cliAddr, sizeof(struct sockaddr_in)) == 0) {
		// 	printf("%d_ok\n",i);
		// }

		// if (memcmp(&(sess[i].cliAddr), &cliAddr, sizeof(struct sockaddr_in)) == 0 && sess[i].connd == connd)
		if(sess[i].connd == connd)
		{
			// printf("\t__Session pos: %d\n", sess[i].connd);
			
			return i;
		}
	}
	return -1;
}

// REFRACTOR find room by id, return room position
int findRoomById(int id)
{
	int i;
	for (i = 0; i < countRoom; i++)
	{
		if (rooms[i].id == id)
		{
			return i;
		}
	}
	return -1;
}

//find room by name, return room position
int findRoomByName(char name[])
{
	int i;
	for (i = 0; i < countRoom; i++)
	{
		if (strcmp(rooms[i].name, name) == 0)
		{
			return i;
		}
	}
	return -1;
}

//remove room
void removeRoom(int k)
{
	int i;
	for (i = k; i < countRoom; ++i)
	{
		rooms[k] = rooms[k + 1];
	}
	countRoom--;
}

// REFRACTOR Xoa cac room ko co nguoi.
void checkListRoom()
{
	for (int i = 0; i < countRoom; ++i)
	{
		if (rooms[i].countUser <= 0)
		{
			printf("remove room:%d\n", i);
			removeRoom(i);
		}
	}
}

int checkPass(char pass[])
{
	//Password has min_length is 5
	int i;
	printf("pass: %s_%d\n", pass, strlen(pass));
	if (strlen(pass) < 5)
		return 0;
	else
		return 1;
}

//create capcha code, include 6 random character
char *makeCapcha()
{
	char *capcha = (char *)malloc(6 * sizeof(char));
	int i;
	srand(time(NULL));
	for (i = 0; i < 6; ++i)
	{
		capcha[i] = '0' + rand() % 72; // starting on '0', ending on '}'
	}
	capcha[6] = '\0';
	return capcha;
}

//find sessionSignup by cliAddr, return session position
int findSessSignByAddr(struct sockaddr_in cliAddr, int connd)
{
	int i = 0;
	for (i = 0; i < sessSignCount; i++)
	{
		if (memcmp(&(sessSignup[i].cliAddr), &cliAddr, sizeof(struct sockaddr_in)) == 0 && sessSignup[i].connd == connd)
		{
			return i;
		}
	}
	return -1;
}

void getListRoom(char message[])
{
	strcat(message, "List room :");
	for (int i = 0; i < countRoom; ++i)
	{
		strcat(message, "\nRoom id :");
		char str[10];
		sprintf(str, "%d", rooms[i].id);
		strcat(message, str);
		strcat(message, "\tPeople count : ");
		sprintf(str, "%d", rooms[i].countUser);
		strcat(message, str);
		strcat(message, " Choose the room or create new room?\n");
	}
}

void getListRoomToken(char message[])
{
	char str[10];
	strcat(message, "_");
	sprintf(str, "%d", countRoom);
	strcat(message, str);
	for (int i = 0; i < countRoom; ++i)
	{
		// strcat(message, "\nRoom id :");
		strcat(message, "_");
		strcat(message, rooms[i].name);
		// sprintf(str, "%d",rooms[i].id);
		// strcat(message,str);
		strcat(message, "_");
		sprintf(str, "%d", rooms[i].countUser);
		strcat(message, str);
		strcat(message, "_");
		sprintf(str, "%d", rooms[i].roomStatus);
		strcat(message, str);
	}
}

char *initSession(struct sockaddr_in cliAddr, int connd)
{
	int pos;
	struct Session session;
	struct User user;

	pos = findSessByAddr(cliAddr, connd);
	// pos = -1;
	if (pos == -1)
	{
		user = newUser(UNKNOWN, "", ACTIVE);
		session = newSession(user, WAIT_FOR_REQUEST, cliAddr, connd); // create new session
		addSession(session);
		pos = findSessByAddr(cliAddr, connd);
		printStatus("SINIT", pos);
		return C_INIT_SUCESS;
	}
	printf("==>Init error!\n");
	return "==>Init error!\n";
}

//process while Code is ALERT
char *alertCodeProcess(char messAcgument[], struct sockaddr_in cliAddr, int connd, int pos)
{
	//found session
	if (atoi(messAcgument) == WAIT_FOR_USERNAME_SIGNUP && userCount == MAX_USER)
	{
		return C_MAX_USER;
	}
	sess[pos].sessStatus = atoi(messAcgument); //WAIT_FOR_USERNAME_SIGNUP
	printStatus("ALERT", pos);
	return C_CHANGED_STATUS;
}

//process while Code is RESET
char *resetCodeProcess(char messAcgument[], struct sockaddr_in cliAddr, int connd, int pos)
{
	struct User user;
	sess[pos].sessStatus = WAIT_FOR_REQUEST;
	if (strcmp(messAcgument, IN_MENU) == 0)
	{
		user = newUser(UNKNOWN, "", ACTIVE);
		sess[pos].user=NULL;
		// memcpy(&(sess[pos].user), &user, sizeof(struct User));
	}
	printStatus(RESET, pos);
	return C_CHANGED_STATUS;
}

//process while Code is LOGIN
char *userCodeProcess(struct sockaddr_in cliAddr, int connd, int pos, int i)
{
	struct Session session;
	if (i == -1)
		return C_NOT_FOUND_ID; //if not found user
	// if (users[i].userStatus == BLOCKED)	return C_BLOCK; //OPTIONAL: if found user but user blocked
	//found session
	if (isLogin(users[i].id) != -1) 
		return C_IS_LOGIN;
	if (sess[pos].sessStatus == WAIT_FOR_USERNAME_LOGIN)
	{ //found user != user of session
		sess[pos].sessStatus = WAIT_FOR_PASS_LOGIN;
		sess[pos].user = users+i;
		// memcpy(&(sess[pos].user), &users[i], sizeof(struct User));
		printStatus("LOGIN", pos);
		return C_FOUND_ID;
	}
	return "Sequence Is Wrong";
}

//Process while Code is UPASS
char *passCodeProcess(char messAcgument[], int pos)
{
	int i;
	// OPTIONAL
	// if (sess[pos].user.userStatus == BLOCKED)
	// 	{
	// 		return C_BLOCK;
	// 	}
	//if PASS ok
	if (strcmp(sess[pos].user->password, messAcgument) == 0)
	{
		sess[pos].sessStatus = WAIT_FOR_REQUEST; //LOGIN SUCESS
		sess[pos].countLogin = 0;				 // reset count login
		sess[pos].isLogin = USER_LOGIN;
		printStatus("UPASS", pos);
		return C_FOUND_PASSWORD;
	}
	else
		return C_NOT_FOUND_PASSWORD;
	return "Sequence Is Wrong";
	//PASS error
	// OPTIONAL
	// else
	// {
	// 	sess[pos].countLogin++;    			//countLogin + 1
	// 	if (sess[pos].countLogin >= MAX_NUMBER_LOGIN){  //check count login is > max?
	// 		sess[pos].user.userStatus = BLOCKED;	//block user
	// 		i = findUserById (sess[pos].user.id);
	// 		users[i].userStatus = BLOCKED;
	// 		writeUserToFile(FILE_NAME);			//save to file
	// 		return C_BLOCK;
	// 	} else
	// 		return C_NOT_FOUND_PASSWORD;
	// }
}

//to do diff
//Process while Code is CROOM, create a new room
char *crrmCodeProcess(char messAcgument[], int pos)
{
	struct Room room;
	int i = findRoomByName(messAcgument);
	if (i >= 0)
	{
		return C_CRE_ROOM_FAI; //room exist
	}
	else
	{
		room = newRoom(messAcgument, sess[pos].connd, *sess[pos].user);
		sess[pos].room = &room;
		sess[pos].sessStatus = WAIT_FOR_REQUEST;
		printStatus(CROOM, pos);
		return C_CRE_ROOM_SUC;
	}
	return "Sequence Is Wrong";
}

//add User to room, i - position of rooms
int addUserToRoom(int i, int connd,struct User user)
{
	int count = rooms[i].countUser;
	if (count <= MAX_USER)
	{
		rooms[i].users[count] = &user;
		rooms[i].connd[count] = connd;
		rooms[i].countUser ++;
		printf("countUser:%d\n", rooms[i].countUser);
		return 1;
	}else return 0;
}

//Process while Code is UJOIN
char *joinCodeProcess(char messAcgument[], int pos, int connd)
{
	//in ra nguoi choi ... da vao phong ...
	//them nguoi choi vao room nay
	int i = findRoomByName(messAcgument);

	if (i < 0 ) //phong khong ton tai
		return C_NON_EXIST_ROOM;
 	if (rooms[i].roomStatus == PLAY) //phong dang choi
		return C_PLAYING_ROOM;
	if (rooms[i].countUser == MAX_USER_IN_ROOM)
		return C_MAX_USER_IN_ROOM;
	if (addUserToRoom(i, connd, *sess[pos].user))
	{
		sess[pos].room = &rooms[i];
		sess[pos].sessStatus = WAIT_FOR_REQUEST;
		printStatus(UJOIN,pos);
		// strcpy()
		// respond(sess[pos].room.connd[0], );
		return C_IN_ROOM;
	}
	return "Sequence Is Wrong";
}

//Process while Code is LOGOU
char *loutCodeProcess(char messAcgument[], int pos)
{
	if (strcmp(sess[pos].user->id, messAcgument) == 0) //check userId is valid?
	{
		struct User user;
		sess[pos].sessStatus = WAIT_FOR_REQUEST;
		sess[pos].isLogin = USER_NOT_LOGIN;
		user = newUser(UNKNOWN, "", ACTIVE);
		sess[pos].user=NULL;
		// memcpy(&(sess[pos].user), &user, sizeof(struct User));
		printStatus(LOGOU, pos);
		return C_LOGOUT_OK;
	}
	else
		return C_LOGOUT_FAILS;
	return "Sequence Is Wrong";
}

//process while code is SIGNU
char *siguCodeProcess(char messAcgument[], struct sockaddr_in cliAddr, int connd, int pos, int i)
{
	struct Session session;
	struct User user;
	if (userCount == MAX_USER)
		return C_MAX_USER; //If max user
	if (i != -1)
		return C_SAME_USER; //if not found user
	// user_temp = newUser(messAcgument, "", ACTIVE);
	strcpy(sess[pos].temp,messAcgument);
	// printf("%d)\n", sess[pos].sessStatus);
	if (sess[pos].sessStatus == WAIT_FOR_USERNAME_SIGNUP)
	{ //found user != user of session
		sess[pos].sessStatus = WAIT_FOR_PASS_SIGNUP;
		sess[pos].user=&user_temp;
		// memcpy(&(sess[pos].user), &user, sizeof(struct User));
		printStatus("SIGNU", pos);
		return C_NEW_USER;
	}
	return "Sequence Is Wrong";
}

//Process while Code is SIGNP
char *sigpCodeProcess(char messAcgument[], int pos)
{
	struct User user;
	// printf("Vao 11\n");
	if (checkPass(messAcgument))
	{

		strcpy(sess[pos].user->password, messAcgument);
		sess[pos].user->count = userCount + 1;

		// NOTE: Da chan truong hop max_user o signup process
		if (addUser(*sess[pos].user))
		{
			writeUserToFile(FILE_NAME);
			sess[pos].sessStatus = WAIT_FOR_REQUEST; //SIGNUP SUCESS //back to default status
			user = newUser(UNKNOWN, "", ACTIVE);	 //reset sess.user
			memcpy(&(sess[pos].user), &user, sizeof(struct User));
			printStatus("SIGNP", pos);
			return C_CORRECT_PASS;
		}
	}
	else
		return C_INCORRECT_PASS;
	return "Sequence Is Wrong";
}

//Process while Code is SIGNC
// char *sigcCodeProcess(char messAcgument[], int pos)
// {
// 	if (strcmp(sessSignup[pos].capcha, messAcgument) == 0) //check capcha
// 	{
// 		sessSignup[pos].sessStatus = SIGNUP_SUCCESSFUL;
// 		sessSignup[pos].user.count = userCount;
// 		if (addUser(sessSignup[pos].user))
// 		{
// 			writeUserToFile(FILE_NAME);
// 			return C_CORRECT_CODE;
// 		}
// 	}
// 	else
// 		return C_INCORRECT_CODE;
// }

//Process while Code is GINFO
// char *ginfoCodeProcess(char messAcgument[], int pos){
// 	int i;
// 	i=findUserInSessRoom(messAcgument,pos);
// 	if (i==0) return sess[pos].room.message1; // firt user
// 	else return sess[pos].room.message2; // second user
// }

//process request
char *process(char messCode[], char messAcgument[], struct sockaddr_in cliAddr, int connd)
{
	int pos, posSign, i;
	// printf("\tSession cond before: %d\n", connd);
	if (strcmp(messCode, SINIT) != 0)
	{
	pos = findSessByAddr(cliAddr, connd); 
	//find Session return -1 if session not exists
	posSign = findSessSignByAddr(cliAddr, connd);

	// //test
	printf("\tSession pos: %d\n", pos);
	printf("\tSession status: %d\n", sess[pos].sessStatus);
	printf("\tSession is login: %d\n", sess[pos].isLogin);
	printf("\tSession condcond: %d\n", sess[pos].connd);
	}

	// checkListRoom(); //todo diff

	/***********messcode is SINIT***********/
	// Khoi tao status ben server
	if (strcmp(messCode, SINIT) == 0)
	{
		return initSession(cliAddr, connd);
	}

	/***********messcode is ALERT***********/
	// Change status ben server

	if (strcmp(messCode, ALERT) == 0)
	{
		return alertCodeProcess(messAcgument, cliAddr, connd, pos);
	}

	/***********messcode is RESET***********/
	// Reset status ben server
	if (strcmp(messCode, RESET) == 0)
	{
		return resetCodeProcess(messAcgument, cliAddr, connd, pos);
	}
	/***********messcode is LOGIN***********/
	if (strcmp(messCode, LOGIN) == 0)
	{
		i = findUserById(messAcgument); //find user return -1 if user not exists
		return userCodeProcess(cliAddr, connd, pos, i);
	}

	/********messcode is UPASS**********/
	if (strcmp(messCode, UPASS) == 0 && sess[pos].sessStatus == WAIT_FOR_PASS_LOGIN)
	{
		return passCodeProcess(messAcgument, pos);
	}

	/********messcode is SIGNU*********/
	if (strcmp(messCode, SIGNU) == 0)
	{
		i = findUserById(messAcgument); //find user return -1 if user not exists
		return siguCodeProcess(messAcgument, cliAddr, connd, pos, i);
	}

	/********messcode is SIGNP*********/
	if (strcmp(messCode, SIGNP) == 0 && sess[pos].sessStatus == WAIT_FOR_PASS_SIGNUP)
	{
		return sigpCodeProcess(messAcgument, pos);
	}

	// /********messcode is SIGNC*********/
	// if (strcmp(messCode, SIGNC) == 0 && posSign != -1 && sessSignup[posSign].sessStatus == PASSWORD_CREATED)
	// {
	// 	return sigcCodeProcess(messAcgument, posSign);
	// }

	/********messcode is LOGOU*********/
	if (strcmp(messCode, LOGOU) == 0 && sess[pos].sessStatus == WAIT_FOR_USERNAME_LOGOUT)
	{
		return loutCodeProcess(messAcgument, pos);
	}
	/********messcode is CROOM**********/
	if (strcmp(messCode, CROOM) == 0 && sess[pos].sessStatus == WAIT_FOR_RNAME_CREATE_ROOM)
	{
		return crrmCodeProcess(messAcgument, pos);
	}
	if (strcmp(messCode, SROOM) == 0 && sess[pos].sessStatus == WAIT_FOR_SEE_ROOM)
	{
		return C_SEE_ROOM_RESULT;
	}
	if (strcmp(messCode, UJOIN) == 0 && sess[pos].sessStatus == WAIT_FOR_RNAME_JOIN_ROOM)
	{
		return joinCodeProcess(messAcgument,pos,connd);
	}
	else
	{
		return "Process Sequence Is Wrong";
	}
}

//convert to full message
void changeFull(char message[],struct sockaddr_in cliAddr, int connd)
{
	int pos=findSessByAddr(cliAddr,connd);
	if (strcmp(message, C_SEE_ROOM_RESULT) == 0)
	{
		getListRoomToken(message);
	}
	if (strcmp(message, C_IN_ROOM) == 0)
	{
		strcat(message,"_");
		strcat(message,sess[pos].room->users[0]->id);
	}
	// if (strcmp(message, C_FOUND_PASSWORD) == 0)
	// {
	// 	strcat(message, " -> Password ok. Login successful!\n");
	// 	// to do diff. for more info
	// }
	// if (strcmp(message, C_CORRECT_PASS) == 0)
	// {
	// 	char capcha[6];
	// 	strcpy(capcha, sessSignup[posCapchar].capcha);
	// 	printf("cap:%s\n",capcha);
	// 	strcat(message, " -> Please enter capcha code : ");
	// 	strcat(message, capcha);
	// }
}

int main(int argc, char *argv[])
{
	if (argc != 2)
		return -1;
	int PORT = atoi(argv[1]);
	char buff[BUFF_SIZE];
	char message[BUFF_SIZE], messCode[BUFF_SIZE], messAcgument[BUFF_SIZE], messTemp[BUFF_SIZE];
	struct pollfd fds[BACKLOG];
	struct sockaddr_in server_addr, client_addr;
	int sin_size = sizeof(client_addr);
	int listen_sock, fdmax, newfd, nbytes, i;

	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Error socket()");
		exit(1);
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);
	if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("Error bind()");
		exit(1);
	}

	if (listen(listen_sock, BACKLOG) == -1)
	{
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
	while (1)
	{
		if (poll(fds, fdmax, -1) == -1)
		{
			perror("Error poll()");
			exit(1);
		}
		for (i = 0; i < fdmax; i++)
		{
			if (fds[i].revents != 0)
			{
				if (fds[i].fd == listen_sock)
				{
					if ((newfd = accept(listen_sock, (struct sockaddr *)&client_addr, (socklen_t *)&sin_size)) < 0)
					{
						perror("Error accept()");
					}
					else
					{
						if (newfd > fdmax)
						{
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
				else if (fds[i].revents & POLLIN)
				{
					readFileUser(FILE_NAME);
					// showUser();
					//recieve data
					if ((nbytes = recv(fds[i].fd, buff, BUFF_SIZE, 0)) <= 0)
					{
						if (nbytes == 0)
							printf("Server: socket %d out\n", fds[i].fd);
						close(fds[i].fd);
					}

					else
					{
						buff[nbytes] = '\0';

						if (isValidMessage(buff, messCode, messAcgument))
						{
							printf("-----------------------------------------------------------\n");
							printf("RUNNING\n\tmessCode:%s\n\tmessAcgument:%s\n", messCode, messAcgument);

							strcpy(message, process(messCode, messAcgument, client_addr, fds[i].fd));

							printf("\t\t==>Return to client: %s\n", message);
							changeFull(message,client_addr,fds[i].fd);
						}
						else
						{
							strcpy(message, "Syntax Error!");
						}
						//send data
						if (strcmp(message, "NULL") != 0)
						{
							respond(fds[i].fd, message);
							bzero(buff, BUFF_SIZE);
						}
						else
						{
							printf("NULL\n");
						}
					}
				}
			}
		}
	}
	return 0;
}