#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "chatroom.h"
#include <poll.h>

#define MAX 1024					 // max buffer size
#define PORT 6789					 // server port number
#define MAX_USERS 50				 // max number of users
static unsigned int users_count = 0; // number of registered users

static user_info_t *listOfUsers[MAX_USERS] = {0}; // list of users

/* Add user to userList */
void user_add(user_info_t *user);
/* Get user name from userList */
char *get_username(int sockfd);
/* Get user sockfd by name */
int get_sockfd(char *name);

/* Add user to userList */
void user_add(user_info_t *user)
{
	if (users_count == MAX_USERS)
	{
		printf("sorry the system is full, please try again later\n");
		return;
	}
	/***************************/
	/* add the user to the list */
	/**************************/
	// listOfUsers[users_count]->sockfd = user->sockfd;
	// strcpy(listOfUsers[users_count]->username, user->username);
	// listOfUsers[users_count]->state = user->state;
	listOfUsers[users_count] = user;
	users_count++;
}

/* Determine whether the user has been registered  */
int isNewUser(char *name)
{
	int i;
	int flag = -1;
	/*******************************************/
	/* Compare the name with existing usernames */
	/*******************************************/

	for (i = 0; i < users_count; i++)
	{
		if (strcmp(listOfUsers[i]->username, name) == 0)
		{
			flag = 1;
			break;
		}
	}

	return flag;
}

/* Get user name from userList */
char *get_username(int ss)
{
	int i;
	static char uname[MAX];
	/*******************************************/
	/* Get the user name by the user's sock fd */
	/*******************************************/
	for (i = 0; i < users_count; i++)
	{
		if (listOfUsers[i]->sockfd == ss)
		{
			strcpy(uname, listOfUsers[i]->username);
			break;
		}
	}
	// do we need to suppose ss must exists in the array?

	printf("get user name: %s\n", uname);
	return uname;
}

/* Get user sockfd by name */
int get_sockfd(char *name)
{
	int i;
	int sock;
	/*******************************************/
	/* Get the user sockfd by the user name */
	/*******************************************/
	sock = -1;
	for (i = 0; i < users_count; i++)
	{
		if (strcmp(listOfUsers[i]->username, name) == 0)
		{
			sock = listOfUsers[i]->sockfd;
			break;
		}
	}

	return sock;
}
// The following two functions are defined for poll()
// Add a new file descriptor to the set
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
	// If we don't have room, add more space in the pfds array
	if (*fd_count == *fd_size)
	{
		*fd_size *= 2; // Double it

		*pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
	}

	(*pfds)[*fd_count].fd = newfd;
	(*pfds)[*fd_count].events = POLLIN; // Check ready-to-read

	(*fd_count)++;
}
// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
	// Copy the one from the end over this one
	pfds[i] = pfds[*fd_count - 1];

	(*fd_count)--;
}

int main()
{
	int listener;  // listening socket descriptor
	int newfd;	   // newly accept()ed socket descriptor
	int addr_size; // length of client addr
	struct sockaddr_in server_addr, client_addr;

	char buffer[MAX]; // buffer for client data
	int nbytes;
	int fd_count = 0;
	int fd_size = 5;
	struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

	int yes = 1; // for setsockopt() SO_REUSEADDR, below
	int i, j, u, rv;

	/**********************************************************/
	/*create the listener socket and bind it with server_addr*/
	/**********************************************************/
	listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener == -1)
	{
		printf("Socket creation failed...\n");
		exit(1);
	}
	else
		printf("Socket successfully created...\n");

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	if (bind(listener, (struct sockaddr *)&server_addr, sizeof server_addr) != 0)
	{
		printf("Socket bind failed...\n");
		exit(1);
	}
	else
		printf("Socket successfully binded.\n");

	// Now server is ready to listen and verification
	if ((listen(listener, 5)) != 0)
	{
		printf("Listen failed...\n");
		exit(3);
	}
	else
		printf("Server listening..\n");

	// Add the listener to set
	pfds[0].fd = listener;
	pfds[0].events = POLLIN; // Report ready to read on incoming connection
	fd_count = 1;			 // For the listener

	// main loop
	for (;;)
	{
		/***************************************/
		/* use poll function */
		/**************************************/
		if (poll(pfds, fd_count, -1) == -1)
		{
			perror("poll");
			exit(1);
		}

		// run through the existing connections looking for data to read
		for (i = 0; i < fd_count; i++)
		{
			if (pfds[i].revents & POLLIN)
			{ // we got one!!
				if (pfds[i].fd == listener)
				{
					/**************************/
					/* we are the listener and we need to handle new connections from clients */
					/****************************/
					addr_size = sizeof(client_addr);
					newfd = accept(listener, (struct sockaddr *)&client_addr, &addr_size);
					if (newfd == -1)
					{
						perror("accept");
					}
					else
					{
						add_to_pfds(&pfds, newfd, &fd_count, &fd_size);
						printf("pollserver: new connection from %s on socket: %d\n", inet_ntoa(client_addr.sin_addr), newfd);

						// send welcome message
						bzero(buffer, sizeof(buffer));
						strcpy(buffer, "Welcome to the chat room!\nPlease enter a nickname.\n");
						if (send(newfd, buffer, sizeof(buffer), 0) == -1)
							perror("send");
					}
				}
				else
				{
					// handle data from a client
					bzero(buffer, sizeof(buffer));
					nbytes = recv(pfds[i].fd, buffer, sizeof(buffer), 0);
					if (nbytes <= 0)
					{
						// got error or connection closed by client
						if (nbytes == 0)
						{
							// connection closed
							printf("pollserver: socket %d hung up\n", pfds[i].fd);
						}
						else
							perror("recv");
						close(pfds[i].fd); // Bye!
						del_from_pfds(pfds, i, &fd_count);
					}
					else
					{
						// we got some data from a client
						if (strncmp(buffer, "REGISTER", 8) == 0)
						{
							printf("Got register/login message\n");
							/********************************/
							/* Get the user name and add the user to the userlist*/
							/**********************************/
							// char *name[strlen(buffer-1)] = (char *)malloc(strlen(buffer) -7);
							char name[MAX];
							strcpy(name, buffer + 8);

							if (isNewUser(name) == -1)
							{
								/********************************/
								/* it is a new user and we need to handle the registration*/
								/**********************************/
								user_info_t *new_user = (user_info_t *)malloc(sizeof(user_info_t));
								if (new_user == NULL)
								{
									perror("Failed to allocate memory for new user");
									exit(1);
								}
								new_user->sockfd = pfds[i].fd;
								new_user->state = 1;
								strcpy(new_user->username, name);

								user_add(new_user);

								/********************************/
								/* create message box (e.g., a text file) for the new user */
								/**********************************/
								FILE *fp;
								char filetype[] = ".txt";
								char filename[MAX];
								strcpy(filename, name);
								strcat(filename, filetype);
								fp = fopen(filename, "w");
								fclose(fp);

								// broadcast the welcome message (send to everyone except the listener)
								bzero(buffer, sizeof(buffer));
								strcpy(buffer, "Welcome ");
								strcat(buffer, name);
								strcat(buffer, " to join the chat room!\n");
								/*****************************/
								/* Broadcast the welcome message*/
								/*****************************/
								for (int ii = 0; ii < fd_count; ii++)
								{
									if (pfds[ii].fd != listener && send(pfds[ii].fd, buffer, sizeof(buffer), 0) == -1)
										perror("send welcome message failed");
								}

								/*****************************/
								/* send registration success message to the new user*/
								/*****************************/
								bzero(buffer, sizeof(buffer));
								strcpy(buffer, "A new account has been created.\n");
								// strcpy(buffer, "Set your password.\n");
								if (send(pfds[i].fd, buffer, sizeof(buffer), 0) == -1)
									perror("send new account message failed");
								else
								{
								}
							}
							else
							{
								/********************************/
								/* it's an existing user and we need to handle the login. Note the state of user,*/
								/**********************************/
								// int login_fd = get_sockfd(name);
								// if (login_fd == -1)
								// {
								// 	perror("login_fd not found.\n");
								// }
								printf("enter existing user\n");
								for (int ii = 0; ii < users_count; ii++)
								{
									if (strcmp(listOfUsers[j]->username, name) == 0)
									{
										listOfUsers[ii]->state = 1;
										listOfUsers[ii]->sockfd = pfds[i].fd;
										break;
									}
								}
								printf("existing user\n");

								/********************************/
								/* send the offline messages to the user and empty the message box*/
								/**********************************/
								FILE *fp;
								char filetype[] = ".txt";
								char filename[MAX];
								strcpy(filename, name);
								strcat(filename, filetype);
								fp = fopen(filename, "r");
								if (fp == NULL)
								{
									perror("no file found");
									exit(1);
								}

								char offline_buffer[MAX];
								char welcome_back[MAX];
								strcpy(welcome_back, "Welcome back! The message box contains:\n");
								if (send(pfds[i].fd, welcome_back, strlen(welcome_back), 0) == -1)
								{
									perror("send welcome back failed");
								}
								while (fgets(offline_buffer, MAX, fp))
								{
									if (send(pfds[i].fd, offline_buffer, strlen(offline_buffer), 0) == -1)
										perror("send offline_buffer failed...");
									// bzero(offline_buffer, sizeof(offline_buffer));
								};
								fclose(fp);

								// broadcast the welcome message (send to everyone except the listener)
								bzero(buffer, sizeof(buffer));
								strcat(buffer, name);
								strcat(buffer, " is online!\n");
								/*****************************/
								/* Broadcast the welcome message*/
								/*****************************/
								for (int ii = 0; ii < fd_count; ii++)
								{
									if (pfds[ii].fd != listener && pfds[ii].fd != pfds[i].fd)
									{
										if (send(pfds[ii].fd, buffer, sizeof(buffer), 0) == -1)
											perror("send welcome message failed");
									}
								}
							}
						}
						else if (strncmp(buffer, "EXIT", 4) == 0)
						{
							printf("Got exit message. Removing user from system\n");
							// send leave message to the other members
							bzero(buffer, sizeof(buffer));
							strcpy(buffer, get_username(pfds[i].fd));
							strcat(buffer, " has left the chatroom\n");
							/*********************************/
							/* Broadcast the leave message to the other users in the group*/
							/**********************************/
							for (int ii = 0; ii < fd_count; ii++)
							{
								if (pfds[ii].fd != listener && pfds[ii].fd != pfds[i].fd)
								{
									if (send(pfds[ii].fd, buffer, sizeof(buffer), 0) == -1)
										perror("send leave message failed");
								}
							}

							/*********************************/
							/* Change the state of this user to offline*/
							/**********************************/
							for (int ii = 0; ii < users_count; ii++)
							{
								if (listOfUsers[ii]->sockfd == pfds[i].fd)
								{
									listOfUsers[ii]->state = 0;
									listOfUsers[ii]->sockfd = 0;
									break;
								}
							}

							// close the socket and remove the socket from pfds[]
							close(pfds[i].fd);
							del_from_pfds(pfds, i, &fd_count);
						}
						else if (strncmp(buffer, "WHO", 3) == 0)
						{
							// concatenate all the user names except the sender into a char array
							printf("Got WHO message from client.\n");
							char ToClient[MAX];
							bzero(ToClient, sizeof(ToClient));
							/***************************************/
							/* Concatenate all the user names into the tab-separated char ToClient and send it to the requesting client*/
							/* The state of each user (online or offline)should be labelled.*/
							/***************************************/
							int first = 1;
							// curr_name = get_username(pfds[i].fd);

							for (int ii = 0; ii < users_count; ii++)
							{

								printf("name: %s", listOfUsers[ii]->username);
								if (strcmp(listOfUsers[ii]->username, get_username(pfds[i].fd)) != 0)
								{
									if (first)
									{
										strcpy(ToClient, listOfUsers[ii]->username);
										if (listOfUsers[ii]->state == 1)
										{
											strcat(ToClient, "*");
										}
										first = 0;
									}
									else
									{
										strcat(ToClient, "\t");
										strcat(ToClient, listOfUsers[ii]->username);
										if (listOfUsers[ii]->state == 1)
										{
											strcat(ToClient, "*");
										}
									}
								}
							}
							strcat(ToClient, "\n* means this user online");
							if (send(pfds[i].fd, ToClient, sizeof(ToClient), 0) == -1)
							{
								perror("Send all user name failed");
							}
						}
						else if (strncmp(buffer, "#", 1) == 0)
						{
							// send direct message
							// get send user name:
							printf("Got direct message.\n");
							// get which client sends the message
							char sendname[MAX];
							// get the destination username
							char destname[MAX];
							// get dest sock
							int destsock;
							// get the message
							char msg[MAX];
							/**************************************/
							/* Get the source name xx, the target username and its sockfd*/
							/*************************************/
							int stop_step = 0;
							bzero(destname, sizeof(destname));
							for (int ii = 1; ii < strlen(buffer); ii++)
							{
								if (buffer[ii] != ':')
								{
									destname[ii - 1] = buffer[ii];
								}
								else
								{
									stop_step = ii + 1;
								}
							}
							strcpy(msg, buffer + stop_step);
							destname[stop_step - 1] = '\0';
							printf("msg: %s\n", msg);

							destsock = get_sockfd(destname);

							printf("destname: %s\n", destname);
							printf("destsock: %d\n", destsock);
							strcpy(sendname, get_username(pfds[i].fd));
							printf("sendname: %s\n", sendname);

							if (destsock == -1)
							{
								/**************************************/
								/* The target user is not found. Send "no such user..." messsge back to the source client*/
								/*************************************/
								char new_msg[] = "There is no such user. Please check your input format\n";
								if (send(pfds[i].fd, new_msg, sizeof(new_msg), 0) == -1)
								{
									perror("Send no target user failed");
								}
							}
							else
							{
								// The target user exists.
								// concatenate the message in the form "xx to you: msg"
								char sendmsg[MAX];
								strcpy(sendmsg, sendname);
								strcat(sendmsg, " to you: ");
								strcat(sendmsg, msg);

								/**************************************/
								/* According to the state of target user, send the msg to online user or write the msg into offline user's message box*/
								/* For the offline case, send "...Leaving message successfully" message to the source client*/
								/*************************************/
								// printf("%s\n", sendmsg);
								for (int ii = 0; ii < users_count; ii++)
								{
									if (strcmp(listOfUsers[ii]->username, destname) == 0)
									{
										if (listOfUsers[ii]->state == 1)
										{
											if (send(destsock, sendmsg, sizeof(sendmsg), 0) == -1)
											{
												perror("send to target user failed");
											}
										}
										else
										{
											// write into file
											FILE *fp;
											char filetype[] = ".txt";
											char filename[MAX];
											strcpy(filename, destname);
											strcat(filename, filetype);
											printf("%s\n", filename);
											fp = fopen(filename, "a");
											if (fp == NULL)
											{
												perror('Open file...\n');
												exit(1);
											}
											fprintf(fp, sendmsg);
											fclose(fp);

											char leave_msg[] = " is offline. Leaving message successfully";
											char pre[MAX];
											strcpy(pre, destname);
											strcat(pre, leave_msg);
											printf("%s", leave_msg);
											if (send(pfds[i].fd, pre, strlen(pre), 0) == -1)
											{
												perror("leave msg to target user failed");
											}
										}
									}
								}
							}

							// if (send(destsock, sendmsg, sizeof(sendmsg), 0) == -1)
							// {
							// 	perror("send");
							// }
						}
						else
						{
							printf("Got broadcast message from user\n");
							/*********************************************/
							/* Broadcast the message to all users except the one who sent the message*/
							/*********************************************/
							for (int ii = 0; ii < fd_count; ii++)
							{
								if (pfds[ii].fd != pfds[i].fd && pfds[ii].fd != listener)
								{

									if (send(pfds[ii].fd, buffer, sizeof(buffer), 0) == -1)
									{
										perror("Broadcast message to all users failed.\n");
									}
								}
							}
						}
					}
				}								  // end handle data from client
			}									  // end got new incoming connection
			else if (pfds[i].revents & POLLRDHUP) // the socket is disconnected
			{
				for (int ii = 0; ii < users_count; ii++)
				{
					if (listOfUsers[ii]->sockfd == pfds[i].fd)
					{
						listOfUsers[ii]->state = 0;
						listOfUsers[ii]->sockfd = 0;
						break;
					}
				}
				// close the socket and remove the socket from pfds[]
				close(pfds[i].fd);
				del_from_pfds(pfds, i, &fd_count);
			}
		} // end looping through file descriptors
	}	  // end for(;;)

	return 0;
}
