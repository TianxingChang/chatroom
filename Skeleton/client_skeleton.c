#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "chatroom.h"

#define MAX 1024  // max buffer size
#define PORT 6789 // port number

static int sockfd;

void generate_menu()
{
	printf("Hello dear user pls select one of the following options:\n");
	printf("EXIT\t-\t Send exit message to server - unregister ourselves from server\n");
	printf("WHO\t-\t Send WHO message to the server - get the list of current users except ourselves\n");
	printf("#<user>: <msg>\t-\t Send <MSG>> message to the server for <user>\n");
	printf("Or input messages sending to everyone in the chatroom.\n");
}

void recv_server_msg_handler()
{
	/********************************/
	/* receive message from the server and desplay on the screen*/
	/**********************************/

	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	char recv_buffer[MAX];
	for (;;)
	{
		pthread_mutex_lock(&mutex);
		bzero(recv_buffer, sizeof(recv_buffer));

		int nbyte = recv(sockfd, recv_buffer, sizeof(recv_buffer), 0);
		if (nbyte <= 0)
		{
			perror("recv");
			pthread_mutex_unlock(&mutex);
			exit(1);
		}
		printf("%s\n", recv_buffer);
		pthread_mutex_unlock(&mutex);
	}
}

int main()
{
	int n;
	int nbytes;
	struct sockaddr_in server_addr, client_addr;
	char buffer[MAX];

	/******************************************************/
	/* create the client socket and connect to the server */
	/******************************************************/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("Socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created...\n");

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(PORT);
	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
	{
		printf("Connection with the server failed...\n");
		exit(0);
	}
	else
		printf("Connected to the server.\n");

	/***** done ******/

	generate_menu();
	// recieve welcome message to enter the nickname
	bzero(buffer, sizeof(buffer));
	if ((nbytes = recv(sockfd, buffer, sizeof(buffer), 0)) == -1)
	{
		perror("recv");
	}
	printf("%s", buffer);

	/*************************************/
	/* Input the nickname and send a message to the server */
	/* Note that we concatenate "REGISTER" before the name to notify the server it is the register/login message*/
	/*******************************************/
	char pre[MAX];
	strcpy(pre, "REGISTER");
	n = 0;
	while ((buffer[n++] = getchar()) != '\n')
		;
	buffer[n - 1] = '\0';
	char my_name[MAX];
	strcpy(my_name, buffer);

	strcat(pre, buffer);
	strcpy(buffer, pre);
	// send message
	if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
	{
		puts("sending REGISTER/LOGIN failed");
		exit(1); // what number shall i use?
	}
	else
	{
		// print: sent successfully
		printf("Register/Login message sent to server.\n");
	}

	/* done */

	// receive welcome message "welcome xx to joint the chatroom. A new account has been created." (registration case) or "welcome back! The message box contains:..." (login case)
	bzero(buffer, sizeof(buffer));
	if (recv(sockfd, buffer, sizeof(buffer), 0) == -1)
	{
		perror("recv");
	}
	printf("%s", buffer);

	/*****************************************************/
	/* Create a thread to receive message from the server*/
	/* pthread_t recv_server_msg_thread;*/
	/*****************************************************/
	pthread_t recv_server_msg_thread;

	if (pthread_create(&recv_server_msg_thread, NULL, recv_server_msg_handler, NULL) != 0)
	{
		perror("Create thread failed");
		exit(1);
	}

	// chat with the server
	for (;;)
	{
		bzero(buffer, sizeof(buffer));
		n = 0;
		while ((buffer[n++] = getchar()) != '\n')
			;
		if ((strncmp(buffer, "EXIT", 4)) == 0)
		{
			printf("Client Exit...\n");
			/********************************************/
			/* Send exit message to the server and exit */
			/* Remember to terminate the thread and close the socket */
			/********************************************/

			// send message
			if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
			{
				puts("sending exit failed");
				exit(1); // what number shall i use?
			}
			else
			{
				printf("Exit message sent to server\n");
				printf("It's OK to Close the window Now or enter ctrl+c\n");
			}

			close(sockfd);
			// pthread_join(recv_server_msg_thread, NULL);
			// pthread_mutex_destroy(&mutex);
			exit(0);
		}
		else if (strncmp(buffer, "WHO", 3) == 0)
		{
			printf("Getting user list, pls hold on...\n");
			if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
			{
				puts("Sending MSG_WHO failed");
				exit(1);
			}
			printf("If you want to send a message to one of the users, pls send with the format: '#username:message'\n");
		}
		else if (strncmp(buffer, "#", 1) == 0)
		{
			// If the user want to send a direct message to another user, e.g., aa wants to send direct message "Hello" to bb, aa needs to input "#bb:Hello"
			if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
			{
				printf("Sending direct message failed...");
				exit(1);
			}
		}
		else
		{
			/*************************************/
			/* Sending broadcast message. The send message should be of the format "username: message"*/
			/**************************************/
			char pre[MAX];
			strcpy(pre, my_name);
			strcat(pre, ": ");
			strcat(pre, buffer);
			strcpy(buffer, pre);
			if (send(sockfd, buffer, sizeof(buffer), 0) < 0)
			{
				printf("Sending direct message failed...");
				exit(1);
			}
		}
	}
	return 0;
}
