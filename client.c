#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 8192
#define FILE_NAME "account.txt"
#define C_CHANGED_STATUS "99"
#define C_INIT_SUCESS "100"

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
// SS: WAIT_FOR_PASS_LOGIN 22
#define C_FOUND_PASSWORD "220"
#define C_NOT_FOUND_PASSWORD "221"

// Logout Return
// SS: WAIT_FOR_USERNAME_LOGOUT 41
#define C_LOGOUT_OK "410"
#define C_LOGOUT_FAILS "411"

// Unuse
#define C_BLOCK "31"
#define C_CORRECT_CODE "60"
#define C_INCORRECT_CODE "61"

// Client Status
#define MENU 0
#define MENU_LOGGED 3
#define EXIT 'q' //Phai la dau nhay don de so sanh ky tu
#define SIGNUP_ING 11
#define SIGNUP_USERNAME_TYPED 12
#define LOGIN_ING 21
#define LOGIN_USERNAME_TYPED 22
#define LOGOUT_ING 41

//	Send to server for change status
#define IN_MENU "0"
#define SIGNUP_REQUEST "11"
#define LOGIN_REQUEST "21"
#define LOGOUT_REQUEST "41"

#define BLOCKED 0
#define ACTIVE 1
#define MAX 100
#define MAX_USER 10

struct User
{
	char id[30];
	char password[30];
	int userStatus;
	int count;
};

struct User users[MAX_USER];

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
	else
	{
		return respond;
	}
}
int status;
int userCount = 0;
char user_name[BUFF_SIZE + 1]; //temp for everything
int main(int argc, char const *argv[])
{
	int SERVER_PORT;
	char SERVER_ADDR[MAX];
	int client_sock;
	char mess[BUFF_SIZE + 1], buff[BUFF_SIZE + 1], respond[BUFF_SIZE], *head, *title;
	struct sockaddr_in server_addr;
	int msg_len, bytes_sent, bytes_received, check;
	int op;

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
			printf("\nWelcome %s! \n 1. Create Room \n 2. Enter Room\n 3. Logout \n", user_name);
			printf("\nEnter your request:");
			scanf("%d%*c", &op);
			switch (op)
			{
			case 3:
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

		default:
			break;
		}
	}
	close(client_sock);
	return 0;
}