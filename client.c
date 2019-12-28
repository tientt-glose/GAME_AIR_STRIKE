#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 8192
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

#define C_FOUND_ID "00"
#define C_NOT_FOUND_ID "01"
#define C_FOUND_PASSWORD "10"
#define C_NOT_FOUND_PASSWORD "11"
#define C_LOGOUT_OK "20"
#define C_LOGOUT_FAILS "21"
#define C_BLOCK "31"


#define C_CORRECT_CODE "60"
#define C_INCORRECT_CODE "61"

//client
#define MENU 0
#define SIGNUP_ING 11
#define SIGNUP_USERNAME_TYPED 12

//send to server
#define IN_MENU "0"
#define SIGNUP_REQUEST "11"
//status of server

#define BLOCKED 0
#define ACTIVE 1
#define MAX 100

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
	printf("%s|\n", mess);
	requestAndReceive(client_sock, mess, respond);
	printf("\nRespond from server:\n%s\n", makeFull(respond));
	while (1)
	{
		// if (strcmp(status, MENU) == 0)
		// {
		// 	printf("Welcome!")
		// }
		printf("Welcome! \n 1. Sign up \n 2. Sign in\n");
		printf("\nEnter your request:");
		scanf("%d%*c", &op);
		switch (op)
		{
		case 1:
			status = SIGNUP_ING;
			generateNormalPackage(mess, "ALERT", SIGNUP_REQUEST);
			requestAndReceive(client_sock, mess, respond);
			printf("\nRespond from server:\n%s\n", makeFull(respond));

			do
			{
				switch (status)
				{
				case SIGNUP_ING:
					head = "SIGNU";
					title = "Enter your user name (no space)";
					break;
				
				case SIGNUP_USERNAME_TYPED:
					head = "SIGNP";
					title = "Enter your pass (no space)";
					break;

				default:
					break;
				}
				input(title, buff);
				generateNormalPackage(mess, head, buff);
				requestAndReceive(client_sock, mess, respond);
				if (strcmp(respond, C_NEW_USER) == 0 && status == SIGNUP_ING)
				{
					status = SIGNUP_USERNAME_TYPED;
				} 
				else if (strcmp(respond, C_MAX_USER) == 0 && status == SIGNUP_ING) {
					// printf("|%s||%s|\n",respond,C_CORRECT_PASS);
					status = MENU;
				}
				else if (strcmp(respond, C_CORRECT_PASS) == 0 && status == SIGNUP_USERNAME_TYPED) {
					// printf("|%s||%s|\n",respond,C_CORRECT_PASS);
					status = MENU;
				}
				printf("\nRespond from server:\n%s\n", makeFull(respond));
				printf("Status: %d\n",status);
			} while (status != MENU);

			// input("Enter your user name (no space)", buff);
			// generateNormalPackage(mess, "SIGNU", buff);
			// printf("|%s|\n", mess);
			// requestAndReceive(client_sock, mess, respond);
			// printf("\nRespond from server:\n%s\n", makeFull(respond));

			// input("Enter your pass (no space)", buff);
			// generateNormalPackage(mess, "SIGNP", buff);
			// requestAndReceive(client_sock, mess, respond);
			// printf("\nRespond from server:\n%s\n", makeFull(respond));

			// input("Enter your capchacapcha (no space)", buff);
			// generateNormalPackage(mess, "SIGNC", buff);
			// requestAndReceive(client_sock, mess, respond);
			// printf("\nRespond from server:\n%s\n", makeFull(respond));

			break;

		default:
			break;
		}

		printf("\nEnter your request:");
		memset(buff, '\0', (strlen(buff) + 1));
		fgets(buff, BUFF_SIZE, stdin);
		msg_len = strlen(buff);
		if (msg_len == 1)
			break;

		//send message
		if (!request(client_sock, buff))
		{
			printf("message send fails\n");
		}
		else
		{
			printf("\n-----------------------------------------------\n");
		}

		if (!receive(client_sock, respond))
		{
			printf("message receive fails\n");
		}
		else
		{
			printf("\nRespond from server:\n%s\n", makeFull(respond));
			printf("\n-----------------------------------------------\n");
			//To do diff
		}
	}
	close(client_sock);
	return 0;
}