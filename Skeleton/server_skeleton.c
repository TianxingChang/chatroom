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

#define MAX 1024 // max buffer size
#define PORT 6789 // server port number
#define MAX_USERS 50 // max number of users
static unsigned int users_count = 0; // number of registered users

static user_info_t *listOfUsers[MAX_USERS] = {0}; // list of users


/* Add user to userList */
void user_add(user_info_t *user);
/* Get user name from userList */
char * get_username(int sockfd);
/* Get user sockfd by name */
int get_sockfd(char *name);

/* Add user to userList */
void user_add(user_info_t *user){
	if(users_count ==  MAX_USERS){
		printf("sorry the system is full, please try again later\n");
		return;
	}
	/***************************/
	/* add the user to the list */
	/**************************/

}

/* Determine whether the user has been registered  */
int isNewUser(char* name) {
	int i;
	int flag = -1;
	/*******************************************/
	/* Compare the name with existing usernames */
	/*******************************************/




	return flag;
}

/* Get user name from userList */
char * get_username(int ss){
	int i;
	static char uname[MAX];
	/*******************************************/
	/* Get the user name by the user's sock fd */
	/*******************************************/


	printf("get user name: %s\n", uname);
	return uname;
}

/* Get user sockfd by name */
int get_sockfd(char *name){
	int i;
	int sock;
	/*******************************************/
	/* Get the user sockfd by the user name */
	/*******************************************/




	return sock;
}
// The following two functions are defined for poll()
// Add a new file descriptor to the set
void add_to_pfds(struct pollfd* pfds[], int newfd, int* fd_count, int* fd_size)
{
	// If we don't have room, add more space in the pfds array
	if (*fd_count == *fd_size) {
		*fd_size *= 2; // Double it

		*pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
	}

	(*pfds)[*fd_count].fd = newfd;
	(*pfds)[*fd_count].events = POLLIN; // Check ready-to-read

	(*fd_count)++;
}
// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int* fd_count)
{
	// Copy the one from the end over this one
	pfds[i] = pfds[*fd_count - 1];

	(*fd_count)--;
}



int main(){
	int listener;     // listening socket descriptor
	int newfd;        // newly accept()ed socket descriptor
	int addr_size;     // length of client addr
	struct sockaddr_in server_addr, client_addr;
	
	char buffer[MAX]; // buffer for client data
	int nbytes;
	int fd_count = 0;
	int fd_size = 5;
	struct pollfd* pfds = malloc(sizeof * pfds * fd_size);
	
	int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, u, rv;

    
	/**********************************************************/
	/*create the listener socket and bind it with server_addr*/
	/**********************************************************/
	



	// Now server is ready to listen and verification
	if ((listen(listener, 5)) != 0) {
		printf("Listen failed...\n");
		exit(3);
	}
	else
		printf("Server listening..\n");
		
	// Add the listener to set
	pfds[0].fd = listener;
	pfds[0].events = POLLIN; // Report ready to read on incoming connection
	fd_count = 1; // For the listener
	
	// main loop
	for(;;) {
		/***************************************/
		/* use poll function */
		/**************************************/


		// run through the existing connections looking for data to read
        	for(i = 0; i < fd_count; i++) {
            	  if (pfds[i].revents & POLLIN) { // we got one!!
                    if (pfds[i].fd == listener) {
                      /**************************/
					  /* we are the listener and we need to handle new connections from clients */
					  /****************************/


                      


						// send welcome message
						bzero(buffer, sizeof(buffer));
						strcpy(buffer, "Welcome to the chat room!\nPlease enter a nickname.\n");
						if (send(newfd, buffer, sizeof(buffer), 0) == -1)
							perror("send");
                      }
                    } else {
                        // handle data from a client
						bzero(buffer, sizeof(buffer));
                        if ((nbytes = recv(pfds[i].fd, buffer, sizeof(buffer), 0)) <= 0) {
                          // got error or connection closed by client
                          if (nbytes == 0) {
                            // connection closed
                            printf("pollserver: socket %d hung up\n", pfds[i].fd);
                          } else {
                            perror("recv");
                          }
						  close(pfds[i].fd); // Bye!
						  del_from_pfds(pfds, i, &fd_count);
                        } else {
                            // we got some data from a client
							if (strncmp(buffer, "REGISTER", 8)==0){
								printf("Got register/login message\n");
								/********************************/
								/* Get the user name and add the user to the userlist*/
								/**********************************/
								if (isNewUser(name) == -1) {
									/********************************/
									/* it is a new user and we need to handle the registration*/
									/**********************************/

									/********************************/
									/* create message box (e.g., a text file) for the new user */
									/**********************************/


									// broadcast the welcome message (send to everyone except the listener)
									bzero(buffer, sizeof(buffer));
									strcpy(buffer, "Welcome ");
									strcat(buffer, name);
									strcat(buffer, " to join the chat room!\n");
									/*****************************/
									/* Broadcast the welcome message*/
									/*****************************/



									/*****************************/
									/* send registration success message to the new user*/
									/*****************************/
								}
								else {
									/********************************/
									/* it's an existing user and we need to handle the login. Note the state of user,*/
									/**********************************/
									
									/********************************/
									/* send the offline messages to the user and empty the message box*/
									/**********************************/
								


									// broadcast the welcome message (send to everyone except the listener)
									bzero(buffer, sizeof(buffer));
									strcat(buffer, name);
									strcat(buffer, " is online!\n");
									/*****************************/
									/* Broadcast the welcome message*/
									/*****************************/
								}
							}
							else if (strncmp(buffer, "EXIT", 4)==0){
								printf("Got exit message. Removing user from system\n");
								// send leave message to the other members
                                bzero(buffer, sizeof(buffer));
								strcpy(buffer, get_username(pfds[i].fd));
								strcat(buffer, " has left the chatroom\n");
								/*********************************/
								/* Broadcast the leave message to the other users in the group*/
								/**********************************/


								/*********************************/
								/* Change the state of this user to offline*/
								/**********************************/

								
								//close the socket and remove the socket from pfds[]
								close(pfds[i].fd);
								del_from_pfds(pfds, i, &fd_count);
							}
							else if (strncmp(buffer, "WHO", 3)==0){
								// concatenate all the user names except the sender into a char array
								printf("Got WHO message from client.\n");
								char ToClient[MAX];
								bzero(ToClient, sizeof(ToClient));
								/***************************************/
								/* Concatenate all the user names into the tab-separated char ToClient and send it to the requesting client*/
								/* The state of each user (online or offline)should be labelled.*/
								/***************************************/




							}
							else if (strncmp(buffer, "#", 1)==0){
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


								if (destsock == -1) {
									/**************************************/
									/* The target user is not found. Send "no such user..." messsge back to the source client*/
									/*************************************/

								}
								else {
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
								
								}
								
								

								if (send(destsock, sendmsg, sizeof(sendmsg), 0) == -1){
									perror("send");
								}

							}
							else{
								printf("Got broadcast message from user\n");
								/*********************************************/
								/* Broadcast the message to all users except the one who sent the message*/
								/*********************************************/

								
							}   

                        }
                    } // end handle data from client
                  } // end got new incoming connection
                } // end looping through file descriptors
        } // end for(;;) 
		

	return 0;
}
