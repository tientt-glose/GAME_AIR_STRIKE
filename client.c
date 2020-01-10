#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
//#include <stropts.h>
#include <stdbool.h>

#define BUFF_SIZE 8192
#define DELIMITER "_"
#define FILE_NAME "account.txt"

#define C_CHANGED_STATUS "99"
#define C_INIT_SUCESS "100"

// Notification
#define C_NO_INFO "404"
#define C_USER_JOIN "714"
#define C_USER_LEAVE "912"
#define C_USER_READY "1012"
#define C_ALL_USER_READY "1013"
#define C_ALL_DONE_ADD_PLANE "1103"
#define C_ALLY_SHOOT_HIT "1200"
#define C_ALLY_SHOOT_MISS "1201"
#define C_ENEMY_SHOOT_HIT "1300"
#define C_ENEMY_SHOOT_MISS "1301"
#define C_YOU_WIN "1400"
#define C_YOU_LOSE "1401"
// Signup Return
// SS: WAIT_FOR_USERNAME_SIGNUP 11
#define C_SAME_USER "110"
#define C_NEW_USER "111"
#define C_MAX_USER "112"
// SS: WAIT_FOR_PASS 12
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

// Leave Room Return
// SS: WAIT_FOR_LEAVE_ROOM 91
#define C_LEAV_ROOM_FAI "910"
#define C_LEAV_ROOM_SUC "911"

// Ready Return
// SS: WAIT_FOR_READY 101
#define C_READY_SUC "1010"
#define C_ALL_READY_SUC "1011"

//Add plane Return
#define C_WAIT_ADD_PLANE "1100"
#define C_ADD_PLANE_SUCCESS "1101"
#define C_DONE_ADD_PLANE "1102"

//Wait Shoot
#define C_YOUR_TURN_SHOOT "1500"
#define C_WAIT_FOR_SHOOT "1501"

// Unuse
#define C_BLOCK "31"
#define C_CORRECT_CODE "60"
#define C_INCORRECT_CODE "61"
#define BLOCKED 0

// Client Status
#define MENU 0
#define MENU_LOGGED 3
#define MENU_INROOM 8
#define MENU_ADDPLANE 11
#define MENU_WAITSHOOT 12
#define MENU_SHOOT 13
#define MENU_WIN 14
#define MENU_LOSE 15
#define EXIT 'q' //Phai la dau nhay don de so sanh ky tu
#define SIGNUP_ING 11
#define SIGNUP_USERNAME_TYPED 12
#define LOGIN_ING 21
#define LOGIN_USERNAME_TYPED 22
#define LOGOUT_ING 41
#define CREATE_ROOM_ING 51
#define SEE_ROOM_ING 61
#define JOIN_ROOM_ING 71
#define LEAVE_ROOM_ING 91
#define READY_ING 101
#define ADD_PLANE_ING 111
#define WAIT_ENEMY_ADD_PLANE 121
#define WAIT_ENEMY_SHOOT 131
#define SHOOTING 141
//	Send to server for change status
// Cac thong so danh cho viet init, reset trang thai. Voi moi loai se co kieu reset tuong ung.
// Chu yeu phuc vu cho viec nguoi dung bam "q" de exit viec nhap du lieu
#define IN_MENU "0"
#define IN_MENU_LOGGED "3" //Danh cho
// Cac status gui di de bao server
#define SIGNUP_REQUEST "11"
#define LOGIN_REQUEST "21"
#define LOGOUT_REQUEST "41"
#define CREATE_ROOM_REQUEST "51"
#define SEE_ROOM_REQUEST "61"
#define JOIN_ROOM_REQUEST "71"
#define LEAVE_ROOM_REQUEST "91"
#define READY_REQUEST "101"
#define ADD_PLANE_REQUEST "111"
#define SHOOT_PLANE_REQUEST "131"

// Status cua user trong phong va sess[poss] user. Khac voi struct user users
#define ACTIVE 1
#define READY 2

#define NOT_FIRST 0
#define DONE_FIRST 1

//Status cua room
#define WAIT 0
#define PLAY 1

#define MAX 100
#define MAX_USER 1000

////GAME PLAY PART
#define MAP_SIZE 10
#define MAX_PLANE 5

//Field Code
#define F_ALIVE_PLANE 1
#define F_MISSED_SHOT 2
#define F_DEAD_PLANE 3
#define F_EMPTY 0

int tempx = 0, tempy = 0;

struct User
{
	char id[30];
	char password[30];
	int userStatus;
	int count;
};

struct User users[MAX_USER];

int _kbhit()
{
	static const int STDIN = 0;
	static bool initialized = false;

	if (!initialized)
	{
		// Use termios to turn off line buffering
		struct termios term;
		tcgetattr(STDIN, &term);
		term.c_lflag &= ~ICANON;
		tcsetattr(STDIN, TCSANOW, &term);
		setbuf(stdin, NULL);
		initialized = true;
	}

	int bytesWaiting;
	ioctl(STDIN, FIONREAD, &bytesWaiting);
	return bytesWaiting;
}

int request(int client_sock, char message[])
{
	if (send(client_sock, message, strlen(message), 0) > 0)
	{
		return 1;
	}
	else
		return 0;
}

int receive(int client_sock, char respond[])
{
	int bytes_received = recv(client_sock, respond, BUFF_SIZE - 1, 0);
	printf("vao_%d", bytes_received);
	if (bytes_received > 0)
	{
		respond[bytes_received] = '\0';
		return 1;
	}
	else
		return 0;
}

int requestAndReceive(int client_sock, char message[], char respond[])
{
	if (send(client_sock, message, strlen(message), 0) > 0)
	{
		int bytes_received = recv(client_sock, respond, BUFF_SIZE - 1, 0);
		if (bytes_received > 0)
		{
			respond[bytes_received] = '\0';
			return 1;
		}
	}
	return 0;
}

void input(char title[], char buff[])
{
	printf("%s: ", title);
	strcpy(buff, "");
	memset(buff, '\0', (strlen(buff) + 1));
	fgets(buff, BUFF_SIZE, stdin);
	// buff[strlen(buff)-1]='\0'; //Loai bo ky tu \n cuoi xau
}

void generateNormalPackage(char mess[], char messCode[], char buff[])
{
	strcpy(mess, messCode);
	strcat(mess, " ");
	strcat(mess, buff);
	// Neu cuoi xau ma ko co ky tu \n thi them vao "\n" vao cuoi xau do. De server chay dc isValidMessage()
	if (mess[strlen(mess)] != '\n')
	{
		mess[strlen(mess)] = '\n';
		mess[strlen(mess) + 1] = '\0';
	}
}

//read file and save to struct User
// void readFileUser(char filename[])
// {
// 	FILE *f = fopen(filename, "r");
// 	userCount = 0;
// 	int i = 0;
// 	char id[30], password[30], userStatus[2];
// 	struct User user;
// 	if (f == NULL)
// 	{
// 		printf("Can't open file %s!\n", filename);
// 		return;
// 	}
// 	else
// 	{
// 		while (!feof(f))
// 		{
// 			fscanf(f, "%s %s %s\n", id, password, userStatus);
// 			user = newUser(id, password, atoi(userStatus));
// 			users[i] = user;
// 			users[i].count = i + 1; //luu STT
// 			userCount++;
// 			i++;
// 		}
// 	}
// 	fclose(f);
// }

char *makeFull(char respond[])
{
	char *token;
	token = strtok(respond, DELIMITER);
	if (strcmp(respond, C_YOUR_TURN_SHOOT) == 0)
	{
		return "Your turn";
	}
	if (strcmp(respond, C_WAIT_FOR_SHOOT) == 0)
	{
		return "Wait the enemy shoot.";
	}
	if (strcmp(respond, C_YOU_WIN) == 0)
	{
		return "Yeah. You win. Congratulation!";
	}
	if (strcmp(respond, C_YOU_LOSE) == 0)
	{
		return "Oh! You lose. Lucky next time.";
	}
	if (strcmp(respond, C_DONE_ADD_PLANE) == 0)
	{
		return "Done add plane.Please ait for opponent!";
	}
	if (strcmp(respond, C_ADD_PLANE_SUCCESS) == 0)
	{
		return "Add plane Success!";
	}
	if (strcmp(respond, C_WAIT_ADD_PLANE) == 0)
	{
		return "Server known!";
	}
	if (strcmp(respond, C_CHANGED_STATUS) == 0)
	{
		return "Server known!";
	}
	if (strcmp(respond, C_INIT_SUCESS) == 0)
	{
		return "Server known!";
	}
	if (strcmp(respond, C_CORRECT_PASS) == 0)
	{
		return "Password is valid. Signup sucessfully!";
	}
	if (strcmp(respond, C_MAX_USER) == 0)
	{
		return "Too many subscribers. Unable to register more.";
	}
	if (strcmp(respond, C_FOUND_ID) == 0)
	{
		return "Username ok, please enter the password!";
	}
	if (strcmp(respond, C_NOT_FOUND_ID) == 0)
	{
		return "User incorrect, try again";
	}
	if (strcmp(respond, C_IS_LOGIN) == 0)
	{
		return "This account is logged in elsewhere. Please log in with another account.";
	}
	if (strcmp(respond, C_FOUND_PASSWORD) == 0)
	{
		return "Password ok. Login successful!";
	}
	if (strcmp(respond, C_NOT_FOUND_PASSWORD) == 0)
	{
		return "Password incorrect , login fails, try again!";
	}
	if (strcmp(respond, C_LOGOUT_OK) == 0)
	{
		return "Logout successful!";
	}
	if (strcmp(respond, C_LOGOUT_FAILS) == 0)
	{
		return "Logout failed";
	}
	if (strcmp(respond, C_BLOCK) == 0)
	{
		return "User blocked, try again!";
	}
	if (strcmp(respond, C_NEW_USER) == 0)
	{
		return "Create new user, enter pass for new user";
	}
	if (strcmp(respond, C_SAME_USER) == 0)
	{
		return "User exits, please choose other user name";
	}
	if (strcmp(respond, C_INCORRECT_PASS) == 0)
	{
		return "Pass is too short, please enter pass >= 5 character";
	}
	if (strcmp(respond, C_CORRECT_CODE) == 0)
	{
		return "Ok, user is created, Please sign in now!";
	}
	if (strcmp(respond, C_CRE_ROOM_SUC) == 0)
	{
		return "Ok, the room is created!";
	}
	if (strcmp(respond, C_CRE_ROOM_FAI) == 0)
	{
		return "The room id is exists! Create a room with another id!";
	}
	if (strcmp(token, C_SEE_ROOM_RESULT) == 0)
	{
		printf("\n\tROOM LIST \n");
		token = strtok(NULL, DELIMITER); //Number of room
		printf("\nTotal number of rooms: %s\n", token);
		printf("\n\tRoom name\t\tCount User\tStatus\n");
		token = strtok(NULL, DELIMITER); //Name of first room
		while (token != NULL)
		{
			printf("%20s\t\t", token);
			token = strtok(NULL, DELIMITER);
			printf("%2s\t\t", token);
			token = strtok(NULL, DELIMITER);
			printf("%2s\n", token);
			token = strtok(NULL, DELIMITER);
		}
		printf("--------------------------------------------------------\n");
		return "ENDING LISTS\n";
	}
	if (strcmp(token, C_ENEMY_SHOOT_HIT) == 0)
	{

		token = strtok(NULL, DELIMITER);
		tempx = atoi(token);
		token = strtok(NULL, DELIMITER);
		tempy = atoi(token);
		printf("Enemy shoot hitted an ally plane: ");
		printf("%d-%d\n", tempx, tempy);
		return "Please check!\n";
	}
		if (strcmp(token, C_ENEMY_SHOOT_MISS) == 0)
	{
		token = strtok(NULL, DELIMITER);
		tempx = atoi(token);
		token = strtok(NULL, DELIMITER);
		tempy = atoi(token);
		printf("Enemy shoot missed an ally plane: ");
		printf("%d-%d\n", tempx, tempy);
		return "You are lucky.!\n";
	}

	if (strcmp(token, C_IN_ROOM) == 0)
	{
		token = strtok(NULL, DELIMITER); //Name of the other user in room
		printf("There is player %s in this room.", token);
		return "Please Ready to play the game and wait until the other user ready!";
	}
	if (strcmp(respond, C_NON_EXIST_ROOM) == 0)
	{
		return "The room id is not exists! Please create a room with that id.";
	}
	if (strcmp(respond, C_PLAYING_ROOM) == 0)
	{
		return "The room is playing! Can not join.";
	}
	if (strcmp(respond, C_MAX_USER_IN_ROOM) == 0)
	{
		return "The room is full slot! Can not join.";
	}
	if (strcmp(respond, C_READY_SUC) == 0)
	{
		return "Ok! We know you are ready!";
	}
	if (strcmp(respond, C_ALL_READY_SUC) == 0)
	{
		return "Ok! We know you are ready! The other also ready!\n Let's play the game!";
	}
	if (strcmp(respond, C_LEAV_ROOM_SUC) == 0)
	{
		return "Leave room successful!";
	}
	if (strcmp(respond, C_LEAV_ROOM_FAI) == 0)
	{
		return "Can't leave this room!";
	}
	if (strcmp(token, C_USER_JOIN) == 0)
	{
		token = strtok(NULL, DELIMITER); //Name of the other user join the room
		printf("The player %s join this room. ", token);
		return "Please Ready to play the game (if not) and wait until the other user ready!";
	}
	if (strcmp(token, C_USER_READY) == 0)
	{
		token = strtok(NULL, DELIMITER); //Name of the other user join the room
		printf("The player %s is ready. ", token);
		return "Please Ready to play the game (if not)!";
	}
	if (strcmp(token, C_ALL_USER_READY) == 0)
	{
		token = strtok(NULL, DELIMITER); //Name of the other user join the room
		printf("All user is ready. You and %s.\n", token);
		return "Let's play the game!";
	}
	if (strcmp(token, C_USER_LEAVE) == 0)
	{
		token = strtok(NULL, DELIMITER); //Name of the other user join the room
		printf("Oh! The player %s leave this room. ", token);
		return "Please Ready and wait until the other user join and ready!";
	}
	if (strcmp(respond, C_ALL_DONE_ADD_PLANE) == 0)
	{
		return "Set plane done!. Let's play the game.";
	}
	else
	{
		return respond;
	}
}

////GAME PLAY PART

void initField(int Field[][MAP_SIZE], int size)
{
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			Field[i][j] = F_EMPTY; //khoi tao map
		}
	}
}

void printField(int playerField[][MAP_SIZE], int enemyField[][MAP_SIZE], int size)
{
	printf("   ");
	for (int i = 0; i < MAP_SIZE; i++)
	{
		printf("%d_", i);
	}
	printf("      ");
	for (int i = 0; i < MAP_SIZE; i++)
	{
		printf("%d_", i);
	}
	printf("\n");

	for (int i = 0; i < MAP_SIZE; i++)
	{
		printf("%d |", i);
		for (int j = 0; j < MAP_SIZE; j++)
		{
			printf("%d ", playerField[j][i]);
		}
		printf("      ");
		for (int j = 0; j < MAP_SIZE; j++)
		{
			printf("%d ", enemyField[j][i]);
		}

		printf("\n");
	}
}

void addPlane(int x, int y, int playerField[][MAP_SIZE], int mapSize)
{
	playerField[x][y] = F_ALIVE_PLANE;
};

void shootPlane(int x, int y, int enemyField[][MAP_SIZE], int mapSize)
{
	enemyField[x][y] = F_MISSED_SHOT;
}
void hitPlane(int x, int y, int enemyField[][MAP_SIZE], int mapSize)
{
	enemyField[x][y] = F_DEAD_PLANE;
}

void enemyShoot(int x, int y, int playerField[][MAP_SIZE], int mapSize)
{
	if (playerField[x][y] == F_ALIVE_PLANE)
	{
		playerField[x][y] = F_DEAD_PLANE;
	}
	else
	{
		playerField[x][y] = F_MISSED_SHOT;
	}
}
int isValidPlanePosition(int x, int y, int playerField[][MAP_SIZE], int mapSize)
{
	if ((0 > x) || (x >= MAP_SIZE) || (0 > y) || (y >= MAP_SIZE))
	{
		printf("x,y value must be >=0 and <%d", MAP_SIZE);
		return 0;
	}
	if (playerField[x][y] != F_EMPTY)
	{
		return 0;
	}
	return 1;
}

int isValidShootPosition(int x, int y, int playerField[][MAP_SIZE], int mapSize)
{
	if ((0 > x) || (x >= MAP_SIZE) || (0 > y) || (y >= MAP_SIZE))
	{
		printf("x,y value must be >=0 and <%d", MAP_SIZE);
		return 0;
	}
	return 1;
}

int status;
int userCount = 0;
int play_flag = ACTIVE;
int play_first_flag = NOT_FIRST;
char user_name[BUFF_SIZE + 1];
char room_name[BUFF_SIZE + 1];

int main(int argc, char const *argv[])
{
	int SERVER_PORT;
	char SERVER_ADDR[MAX];
	int client_sock;
	char mess[BUFF_SIZE + 1], buff[BUFF_SIZE + 1], respond[BUFF_SIZE], *head, *title, *token, temp[BUFF_SIZE];
	struct sockaddr_in server_addr;
	int msg_len, bytes_sent, bytes_received, check;
	int op;

	int playerField[MAP_SIZE][MAP_SIZE];
	int enemyField[MAP_SIZE][MAP_SIZE];
	initField(playerField, MAP_SIZE);
	initField(enemyField, MAP_SIZE);

	if (argc != 3)
	{
		printf("Close\n");
		exit(1);
	}
	SERVER_PORT = atoi(argv[2]);
	strcpy(SERVER_ADDR, argv[1]);

	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
	{
		printf("\nError!Can not ~! Client exit imediately! ");
		return 0;
	}
	status = MENU;
	generateNormalPackage(mess, "SINIT", IN_MENU);
	requestAndReceive(client_sock, mess, respond);
	printf("\nRespond from server:\n%s\n", makeFull(respond));
	while (1)
	{
		switch (status)
		{
		case MENU:
			printf("\nWelcome! \n 1. Sign up \n 2. Log in\n");
			printf("\nEnter your request:");
			scanf("%d%*c", &op);
			switch (op)
			{
			case 1:
				status = SIGNUP_ING;
				generateNormalPackage(mess, "ALERT", SIGNUP_REQUEST);
				requestAndReceive(client_sock, mess, respond);
				printf("\nRespond from server:\n%s\n", makeFull(respond));

				if (strcmp(respond, C_MAX_USER) == 0)
					status = MENU;

				while (status != MENU)
				{
					switch (status)
					{
					case SIGNUP_ING:
						head = "SIGNU";
						title = "Enter your user name (no space, q for exit)";
						break;

					case SIGNUP_USERNAME_TYPED:
						head = "SIGNP";
						title = "Enter your pass (no space, q for exit)";
						break;

					default:
						break;
					}
					input(title, buff);
					// For q reset status
					if (buff[0] == EXIT && strlen(buff) == 2)
					{
						generateNormalPackage(mess, "RESET", IN_MENU);
						requestAndReceive(client_sock, mess, respond);
						status = MENU;
						break;
					}
					generateNormalPackage(mess, head, buff);
					requestAndReceive(client_sock, mess, respond);
					if (strcmp(respond, C_NEW_USER) == 0 && status == SIGNUP_ING)
					{
						status = SIGNUP_USERNAME_TYPED;
					}
					else if (strcmp(respond, C_MAX_USER) == 0 && status == SIGNUP_ING)
					{
						status = MENU;
					}
					else if (strcmp(respond, C_CORRECT_PASS) == 0 && status == SIGNUP_USERNAME_TYPED)
					{
						status = MENU;
					}
					printf("\nRespond from server:\n%s\n", makeFull(respond));

					printf("Status: %d\n", status);
				}
				break;

			case 2:
				status = LOGIN_ING;
				generateNormalPackage(mess, "ALERT", LOGIN_REQUEST);
				requestAndReceive(client_sock, mess, respond);
				printf("\nRespond from server:\n%s\n", makeFull(respond));

				while (status != MENU_LOGGED)
				{
					switch (status)
					{
					case LOGIN_ING:
						head = "LOGIN";
						title = "Enter your user name (no space)";
						break;

					case LOGIN_USERNAME_TYPED:
						head = "UPASS";
						title = "Enter your password (no space)";
						break;

					default:
						break;
					}
					input(title, buff);
					//For q reset status
					if (buff[0] == EXIT && strlen(buff) == 2)
					{
						generateNormalPackage(mess, "RESET", IN_MENU);
						requestAndReceive(client_sock, mess, respond);
						status = MENU;
						break;
					}
					generateNormalPackage(mess, head, buff);
					requestAndReceive(client_sock, mess, respond);
					printf("\nRespond from server:\n%s\n", makeFull(respond));

					if (strcmp(respond, C_FOUND_ID) == 0 && status == LOGIN_ING)
					{
						status = LOGIN_USERNAME_TYPED;
						strncpy(user_name, buff, strlen(buff) - 1);
					}
					else if (strcmp(respond, C_FOUND_PASSWORD) == 0 && status == LOGIN_USERNAME_TYPED)
					{
						status = MENU_LOGGED; //chuyen cach in menu
					}
				}

				break;

			default:
				break;
			}
			break;

		case MENU_LOGGED:
			printf("\nWelcome %s! \n 1. Create Room \n 2. View Room List \n 3. Join Room \n 4. Logout \n", user_name);
			printf("\nEnter your request:");
			scanf("%d%*c", &op);
			switch (op)
			{
			case 1:
				status = CREATE_ROOM_ING;
				generateNormalPackage(mess, "ALERT", CREATE_ROOM_REQUEST);
				requestAndReceive(client_sock, mess, respond);
				printf("\nRespond from server:\n%s\n", makeFull(respond));
				while (status != MENU_INROOM)
				{
					switch (status)
					{
					case CREATE_ROOM_ING:
						head = "CROOM";
						title = "Enter the room name (no space, q for exit)";
						break;

					default:
						break;
					}
					input(title, buff);
					if (buff[0] == EXIT && strlen(buff) == 2)
					{
						generateNormalPackage(mess, "RESET", IN_MENU_LOGGED);
						requestAndReceive(client_sock, mess, respond);
						status = MENU_LOGGED;
						break;
					}
					generateNormalPackage(mess, head, buff);
					requestAndReceive(client_sock, mess, respond);
					printf("\nRespond from server:\n%s\n", makeFull(respond));

					if (strcmp(respond, C_CRE_ROOM_SUC) == 0 && status == CREATE_ROOM_ING)
					{
						status = MENU_INROOM;
						strncpy(room_name, buff, strlen(buff) - 1);
						// status = CREATE_ROOM_ING;
					}
				}
				break;
			case 2:
				status = SEE_ROOM_ING;
				generateNormalPackage(mess, "ALERT", SEE_ROOM_REQUEST);
				requestAndReceive(client_sock, mess, respond);
				while (status != MENU_LOGGED)
				{
					switch (status)
					{
					case SEE_ROOM_ING:
						head = "SROOM";
						break;

					default:
						break;
					}
					generateNormalPackage(mess, head, SEE_ROOM_REQUEST);
					requestAndReceive(client_sock, mess, respond);
					strcpy(temp, respond);
					token = strtok(temp, DELIMITER);
					printf("\nRespond from server:\n");
					printf("%s\n", makeFull(respond));

					if (strcmp(token, C_SEE_ROOM_RESULT) == 0 && status == SEE_ROOM_ING)
					{
						status = MENU_LOGGED;
					}
					else // DOTO: ban muon gui tai tap tin
					{
						input("test: ", buff);
					}
				}
				break;
			case 3:
				status = JOIN_ROOM_ING;
				generateNormalPackage(mess, "ALERT", JOIN_ROOM_REQUEST);
				requestAndReceive(client_sock, mess, respond);
				printf("\nRespond from server:\n%s\n", makeFull(respond));
				while (status != MENU_INROOM)
				{
					switch (status)
					{
					case JOIN_ROOM_ING:
						head = "UJOIN";
						title = "Enter the room name (no space, q for exit)";
						break;

					default:
						break;
					}
					input(title, buff);
					if (buff[0] == EXIT && strlen(buff) == 2)
					{
						generateNormalPackage(mess, "RESET", IN_MENU_LOGGED);
						requestAndReceive(client_sock, mess, respond);
						status = MENU_LOGGED;
						break;
					}
					generateNormalPackage(mess, head, buff);
					requestAndReceive(client_sock, mess, respond);
					strcpy(temp, respond);
					token = strtok(temp, DELIMITER);
					printf("\nRespond from server:\n");
					printf("%s\n", makeFull(respond));

					if (strcmp(token, C_IN_ROOM) == 0 && status == JOIN_ROOM_ING)
					{
						status = MENU_INROOM;
						strncpy(room_name, buff, strlen(buff) - 1);
					}
				}
				break;
			case 4:
				status = LOGOUT_ING;
				generateNormalPackage(mess, "ALERT", LOGOUT_REQUEST);
				requestAndReceive(client_sock, mess, respond);
				printf("\nRespond from server:\n%s\n", makeFull(respond));
				while (status != MENU)
				{
					switch (status)
					{
					case LOGOUT_ING:
						head = "LOGOU";
						strcpy(buff, user_name);
						strcat(buff, "\n");
						break;

					default:
						break;
					}
					generateNormalPackage(mess, head, buff);
					requestAndReceive(client_sock, mess, respond);
					printf("\nRespond from server:\n%s\n", makeFull(respond));

					if (strcmp(respond, C_LOGOUT_OK) == 0 && status == LOGOUT_ING)
					{
						status = MENU;
					}
				}

				break;

			default:
				break;
			}
			break;

		case MENU_INROOM:
			if (play_flag == ACTIVE)
				printf("\nWelcome %s! In room %s.\n 1. Play Game (ready) \n 2. Leave Room \n", user_name, room_name);
			else
				printf("\nWelcome %s! In room %s.\n 1. You are already ready\n 2. Leave Room \n", user_name, room_name);
			printf("Wait other player join and ready. If you want to make request press any key!\n");
			while (!_kbhit())
			{
				generateNormalPackage(mess, "GINFO", user_name);
				requestAndReceive(client_sock, mess, respond);
				strcpy(temp, respond);
				token = strtok(temp, DELIMITER);
				if (strcmp(token, C_USER_JOIN) == 0 || strcmp(token, C_USER_LEAVE) == 0 || strcmp(token, C_USER_READY) == 0 || strcmp(token, C_ALL_USER_READY) == 0)
				{
					printf("\nRespond from server:\n");
					printf("%s\n", makeFull(respond));
					generateNormalPackage(mess, "RINFO", user_name);
					requestAndReceive(client_sock, mess, respond);
					if (strcmp(token, C_ALL_USER_READY) == 0)
					{
						status = MENU_ADDPLANE;
						break;
					}
				}
			}
			if (status == MENU_INROOM)
			{
				if (play_flag == ACTIVE)
					printf("\nWelcome %s! In room %s.\n 1. Play Game (ready) \n 2. Leave Room \n", user_name, room_name);
				else
					printf("\nWelcome %s! In room %s.\n 1. You are already ready\n 2. Leave Room \n", user_name, room_name);
				printf("\nEnter your request: ");
				scanf("%*c%d%*c", &op);
				switch (op)
				{
				case 1:
					if (play_flag != ACTIVE)
						break;
					status = READY_ING;
					generateNormalPackage(mess, "ALERT", READY_REQUEST);
					requestAndReceive(client_sock, mess, respond);
					printf("\nRespond from server:\n%s\n", makeFull(respond));
					while (status != MENU_ADDPLANE)
					{
						switch (status)
						{
						case READY_ING:
							head = "UREAD";
							strcpy(buff, user_name);
							strcat(buff, "\n");
							break;

						default:
							break;
						}
						generateNormalPackage(mess, head, buff);
						requestAndReceive(client_sock, mess, respond);
						printf("\nRespond from server:\n%s\n", makeFull(respond));

						if (strcmp(respond, C_ALL_READY_SUC) == 0 && status == READY_ING)
						{
							play_flag = READY;
							status = MENU_ADDPLANE;
						}
						else if (strcmp(respond, C_READY_SUC) == 0)
						{
							play_flag = READY;
							status = MENU_INROOM;
							break;
						}
					}
					break;
				case 2:
					status = LEAVE_ROOM_ING;
					generateNormalPackage(mess, "ALERT", LEAVE_ROOM_REQUEST);
					requestAndReceive(client_sock, mess, respond);
					printf("\nRespond from server:\n%s\n", makeFull(respond));
					while (status != MENU_LOGGED)
					{
						switch (status)
						{
						case LEAVE_ROOM_ING:
							head = "LEAVE";
							strcpy(buff, room_name);
							strcat(buff, "\n");
							break;

						default:
							break;
						}
						generateNormalPackage(mess, head, buff);
						requestAndReceive(client_sock, mess, respond);
						printf("\nRespond from server:\n%s\n", makeFull(respond));

						if (strcmp(respond, C_LEAV_ROOM_SUC) == 0 && status == LEAVE_ROOM_ING)
						{
							status = MENU_LOGGED;
							play_flag = ACTIVE;
						}
					}
					break;

				default:
					break;
				}
			}
			break;
		case MENU_ADDPLANE:
			status = ADD_PLANE_ING;
			generateNormalPackage(mess, "ALERT", ADD_PLANE_REQUEST);
			requestAndReceive(client_sock, mess, respond);
			printf("\nRespond from server:\n%s\n", makeFull(respond));
			int addProcess = 0;
			char buff[500];
			do
			{
				printf("Enter plane %d position: (format:x-y or q for exit): ", addProcess);
				scanf("%d%*c%d%*c", &tempx, &tempy);
				printf("%d-%d\n", tempx, tempy);
				if (isValidPlanePosition(tempx, tempy, playerField, MAP_SIZE) == 0)
				{
					continue;
				}
				addPlane(tempx, tempy, playerField, MAP_SIZE);
				addProcess++;
				sprintf(buff, "%d-%d\n", tempx, tempy);
				generateNormalPackage(mess, "ADDPL", buff);
				requestAndReceive(client_sock, mess, respond);
				printf("\nRespond from server:\n%s\n", makeFull(respond));
			} while (addProcess != MAX_PLANE);
			if (strcmp(respond, C_ALL_DONE_ADD_PLANE) == 0)
			{
				status = MENU_WAITSHOOT;
				break;
			}
			while (!_kbhit())
			{
				generateNormalPackage(mess, "GINFO", user_name);
				requestAndReceive(client_sock, mess, respond);
				if (strcmp(respond, C_ALL_DONE_ADD_PLANE) == 0)
				{
					printf("\nRespond from server:\n");
					printf("%s\n", makeFull(respond));
					generateNormalPackage(mess, "RINFO", user_name);
					requestAndReceive(client_sock, mess, respond);
					status = MENU_WAITSHOOT;
					break;
				}
			}
			// input("test", buff);
			break;
		case MENU_WAITSHOOT:
			status = WAIT_ENEMY_SHOOT;
			generateNormalPackage(mess, "ALERT", SHOOT_PLANE_REQUEST);
			requestAndReceive(client_sock, mess, respond);
			printf("\nRespond from server:\n%s\n", makeFull(respond));
			// input("Test",buff);
			// printf("test2");

			if (strcmp(respond, C_YOUR_TURN_SHOOT) == 0)
			{
				status = MENU_SHOOT;
				generateNormalPackage(mess, "RINFO", SHOOT_PLANE_REQUEST);
				requestAndReceive(client_sock, mess, respond);
				break;
			}

			while (!_kbhit())
			{
				generateNormalPackage(mess, "GINFO", user_name);
				requestAndReceive(client_sock, mess, respond);
				strcpy(temp, respond);
				token = strtok(temp, DELIMITER);

				if (strcmp(token, C_YOUR_TURN_SHOOT) == 0)
				{
					printf("\nRespond from server:\n");
					printf("%s\n", makeFull(respond));
					generateNormalPackage(mess, "RINFO", user_name);
					requestAndReceive(client_sock, mess, respond);
					status = MENU_SHOOT;
					break;
				}
				if (strcmp(token, C_ENEMY_SHOOT_HIT) == 0)
				{
					printf("\nRespond from server:\n");
					printf("%s\n", makeFull(respond));
					enemyShoot(tempx, tempy, playerField, MAP_SIZE);
					printField(playerField, enemyField, MAP_SIZE);
					generateNormalPackage(mess, "RINFO", user_name);
					requestAndReceive(client_sock, mess, respond);
					status = MENU_SHOOT;
					break;
				}
				if (strcmp(token, C_ENEMY_SHOOT_MISS) == 0)
				{
					printf("\nRespond from server:\n");
					printf("%s\n", makeFull(respond));
					int tempx = 0, tempy = 0;

					token = strtok(NULL, DELIMITER);
					tempx = atoi(token);
					token = strtok(NULL, DELIMITER);
					tempy = atoi(token);

					enemyShoot(tempx, tempy, playerField, MAP_SIZE);
					generateNormalPackage(mess, "RINFO", user_name);
					requestAndReceive(client_sock, mess, respond);
					status = MENU_SHOOT;
					break;
				}
				if (strcmp(token, C_YOU_LOSE) == 0)
				{

					status = MENU_LOSE;
					break;
				}
			}
			break;
		case MENU_SHOOT:
			status = SHOOTING;
			do
			{
				printf("Enter shoot position: (format:x-y or q for exit): ");
				scanf("%d%*c%d%*c", &tempx, &tempy);
				printf("%d-%d\n", tempx, tempy);
			} while (isValidShootPosition(tempx, tempy, playerField, MAP_SIZE) != 1);
			shootPlane(tempx, tempy, enemyField, MAP_SIZE);
			sprintf(buff, "%d-%d\n", tempx, tempy);
			printf("%s\n", buff);
			generateNormalPackage(mess, "SHOOT", buff);
			requestAndReceive(client_sock, mess, respond);
			// printf("\nRespond from server:\n%s\n", makeFull(respond));

			if (strcmp(respond, C_YOU_WIN) == 0)
			{
				hitPlane(tempx, tempy, enemyField, MAP_SIZE);
				status = MENU_WIN;
				printf("You Win");
				break;
			}
			if (strcmp(respond, C_ALLY_SHOOT_HIT) == 0)
			{
				hitPlane(tempx, tempy, enemyField, MAP_SIZE);
				printf("You hitted enemy plane at:%d-%d\n", tempx, tempy);
			}
			if (strcmp(respond, C_ALLY_SHOOT_MISS) == 0)
			{
				printf("You missed at:%d-%d\n", tempx, tempy);
			}

			printField(playerField, enemyField, MAP_SIZE);
			status = MENU_WAITSHOOT;
			// generateNormalPackage(mess, "ALERT", SHOOT_PLANE_REQUEST);
			// requestAndReceive(client_sock, mess, respond);
			// printf("\nRespond from server:\n%s\n", makeFull(respond));
			break;

		case MENU_WIN:
			printf("\nRespond from server:\n%s\n", makeFull(respond));
			input("win", buff);
			break;
		case MENU_LOSE:
			printf("\nRespond from server:\n%s\n", makeFull(respond));
			generateNormalPackage(mess, "RINFO", user_name);
			requestAndReceive(client_sock, mess, respond);
			generateNormalPackage(mess, "RESET", IN_MENU_LOGGED);
			requestAndReceive(client_sock, mess, respond);
			input("lose", buff);
			break;
		default:
			break;
		}
	}
	close(client_sock);
	return 0;
}