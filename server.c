/*
 * webserver.c
 *
 * Dylan Tokotch 03/07/2015
 */
#include <stdio.h>//standard IO for prints and such
#include <stdlib.h>//standard library for malloc
#include <string.h>//memset
#include <sys/socket.h>//contains SOCK_STREAM and socket structs
#include <netdb.h>//contains the addrinfo struct
#include <pthread.h>//for mutexes
#include <unistd.h>//for shared memory
#include "html.h"

#define HOST_NAME_MAX 256 

int main(int argc, char *argv[]) 
{
	char *hostName;
	//if the host name is given by arguement, then use it
    	if (argc == 2) 
	{
		hostName = argv[1];		
	}
	//else, allocate 256 bytes for the hostName and set it equal to the computer name
	else 
	{
		hostName = malloc(HOST_NAME_MAX);
		memset(hostName, 0, HOST_NAME_MAX);

		if (gethostname(hostName, HOST_NAME_MAX) < 0) 
		{
			printf("Fail: gethostname error!\n");
			return(-1);
		}
	}
	printf("Success: Host name: %s\n", hostName);

	//open the config file, read of the constants, check for their viability, and close the config file.  Display any errors with config file if they exist
	FILE *configPointer = fopen("serverConfig","r");
	if(configPointer == NULL)
	{
		printf("Fail: Could not open config file!\n");
		exit(1);
	}
	int MAX_SIMULTANEOUS_SESSIONS;
	char string[26]; //works as long as the variable names don't get longer	
	int thisNumber;
	while(fscanf(configPointer, "%s %d", string, &thisNumber) != EOF)
	{
		if(strcmp(string, "MAX_SIMULTANEOUS_SESSIONS")==0)
		{
			MAX_SIMULTANEOUS_SESSIONS = thisNumber;
		}
	}
	fclose(configPointer);
	printf("Success: Config File Read!\n");

	//create structs to store hostAddressInfo, and hint to be used later
	struct addrinfo *hostAddressInfo;
	struct addrinfo hint;
	hint.ai_flags = 0;
	hint.ai_family = 0;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_protocol = 0;
	hint.ai_addrlen = 0;
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL;
	hint.ai_next = NULL;

	//get address info and put it in hostAddressInfo
	if((getaddrinfo(hostName, "mywebserver", &hint, &hostAddressInfo)) != 0)
	{
		printf("Fail: getaddrinfo error!\n");
		exit(1);
	}
	printf("Success: getAddrInfo!\n");
	//create file descriptor for hosting and fill it
	int hostFileDescriptor;
	if((hostFileDescriptor = socket(hostAddressInfo->ai_addr->sa_family, SOCK_STREAM, 0)) < 0)
	{
		printf("Fail: socket error!\n");
		exit(1);
	}
	printf("Success: hostFileDescriptor! %d\n", hostFileDescriptor);
	//bind the fileDescriptor
	if(bind(hostFileDescriptor,hostAddressInfo->ai_addr, hostAddressInfo->ai_addrlen) < 0)
	{
		printf("Fail: bind error!\n");
		exit(1);
	}
	printf("Success: bind!\n");
	//free address info
	freeaddrinfo(hostAddressInfo);
	//start listener
	if(listen(hostFileDescriptor, MAX_SIMULTANEOUS_SESSIONS) < 0)
	{
		printf("Fail: listen fail!\n");
		exit(1);
	}
	printf("Success: listen!\n");
	//loop forever
	while(1)
	{
		//main thread loops forever, accepts connects iff we have less than the max number of connections already
		//accept incoming connections and put them in file descriptors		
		int clientFileDescriptor = accept(hostFileDescriptor, NULL, NULL);
		if(clientFileDescriptor < 0)
		{
			printf("Fail: accept fail!\n");
		}
	
		pid_t processID = fork();

		//fork
		if(processID == 0)
		{
			
			//child process will close the host FD socket, service the client's HTML request, and close the client FD socket.
			if(close(hostFileDescriptor) < 0)
			{
				printf("Error closing hostFD in child.\n");
				exit(1);
			}
			//START OF SERVICE BLOCK
			//Get variables setup
			struct ReqInfo requestInfo;
			int resource = 0;
			initRequestInfo(&requestInfo);

			//Get the HTTP request from the client file descriptor
			if(getRequest(clientFileDescriptor, &requestInfo) < 0)
			{
				printf("getRequest returned an error");
				return -1;
			}
		
			//Check to see if requested resource exists
			if(requestInfo.status == 200)
			{
				if((resource = checkResource(&requestInfo)) < 0)
				{
					requestInfo.status = 404;
					printf("File not found!\n");
				}	
			}
			//We have to send HTTP response headers to a full request
			if(requestInfo.type == 1)
			{
				outputHTTPHeaders(clientFileDescriptor, &requestInfo);	
			}
		
			//Send the requested resource
			if(requestInfo.status == 200)
			{
				if(returnResource(clientFileDescriptor, resource, &requestInfo) < 0)
				{
					printf("Something went wrong returning resource.\n");
					return -1;
				}
			}
			else
			{
				Return_Error_Msg(clientFileDescriptor, &requestInfo);
			}
			if(resource > 0)
			{
				if(close(resource) < 0)
				{
					printf("Error closing resource.");
				}
			}
			printf("Successfully serviced %s!\n", requestInfo.resource);
			freeRequestInfo(&requestInfo);
			//END OF SERVICE BLOCK
			if(close(clientFileDescriptor) < 0)
			{
				printf("Error closing clientFD in child.\n");
				exit(1);
			}
			exit(EXIT_SUCCESS);
		}
		else
		{
			//parent process will close socket and return to ready state to accept new connections
			if(close(clientFileDescriptor) < 0)
			{
				printf("Error closing clientFD in parent.\n");
			}
		}
	}
}

