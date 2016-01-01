#include <sys/time.h> //for select()
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h> // for O_RDONLY
#include "html.h"
#include "pgLib.h"

#define ROOT_FOLDER "/home/dylan/OS/Homework/honorsupgrade/HTML"
//Returns requested resource
int returnResource(int fileDescriptor, int resource, struct ReqInfo *requestInfo)
{
	char c;
	int i;
	while((i = read(resource, &c, 1)))
	{
		if(i < 0)
		{
			printf("Error reading from resource.");
			return(-1);
		}
		if(write(fileDescriptor, &c, 1) < 1)
		{
			printf("Error sending file.");
			return(-1);
		}	
	}
	return 0;
}
//checks requested resource
int checkResource(struct ReqInfo *requestInfo)
{
	char root[1000];
	strcpy(root,ROOT_FOLDER);;
	CleanURL(requestInfo->resource);
	strcat(root, requestInfo->resource);	
	return open(root, O_RDONLY);;
}

//Initialises requestInfo struct
void initRequestInfo(struct ReqInfo *requestInfo)
{
	requestInfo->useragent = NULL;
	requestInfo->referer = NULL;
	requestInfo->resource = NULL;
	requestInfo->method = -1;
	requestInfo->status = 200;
}
//Frees memory used by requestInfo struct
void freeRequestInfo(struct ReqInfo *requestInfo)
{
	if(requestInfo->useragent)
		free(requestInfo->useragent);
	if(requestInfo->referer)
		free(requestInfo->referer);
	if(requestInfo->resource)
		free(requestInfo->resource);
}
//This opens a buffer and waits for either the timeout or the complete header, whichever comes first.  It stores the request in a buffer, then calls praseHTTPHeader
int getRequest(int fileDescriptor, struct ReqInfo *requestInfo)
{	
	char buffer[MAX_REQ_LINE] = {0};
	//set timeout value to 5 seconds and 0 microseconds
	struct timeval timeValue;
	timeValue.tv_sec = 25;
	timeValue.tv_usec = 0;
	//create a file descriptor set (no idea how this differs from a file descriptor)
	fd_set fileDescriptorSet;
	do
	{

		FD_ZERO(&fileDescriptorSet);//resets file descriptor set
		FD_SET(fileDescriptor, &fileDescriptorSet);//sets file descriptor set
		int timeWait = select(fileDescriptor + 1, &fileDescriptorSet, NULL, NULL, &timeValue);
		if(timeWait < 0)
		{
			printf("There was an error calling select()\n");
			return -1;
		}
		else if(timeWait == 0)
		{
			printf("Timed out\n");
			return -1;
		}
		else
		{
			Readline(fileDescriptor, buffer, MAX_REQ_LINE -1);
			Trim(buffer);
			if(buffer[0] == '\0')
			{
				break;
			}
			if(parseHTTPHeader(buffer, requestInfo))
			{
				break;
			}
		}
	}while(requestInfo->type == 1);
	return 0;
}	
//Parses a buffered HTTP request and stores it in reqinfo struct
int parseHTTPHeader(char *buffer, struct ReqInfo *requestInfo) 
{
	static int firstHeader = 1;
	int len;
	char *endPointer;
	if(firstHeader == 1)
	{
		//The first line can either start with GET or HEAD
		if(strncmp(buffer, "GET ", 4) == 0)
		{
			requestInfo->method = 0;
			buffer+=4;
		}
		else if(strncmp(buffer, "HEAD ", 5) == 0)
		{
			requestInfo->method = 1;
			buffer+=5;
		}
		else
		{
			requestInfo->method = -1;
			requestInfo->status = 501;
			return -1;
		}
		//Move the buffer through whitespace to the resource
		while(*buffer && isspace(*buffer))
			buffer++;
		//Now we need to put all of resource in our resource string
		endPointer = strchr(buffer,' ');
		if(endPointer == NULL)
			len = strlen(buffer);
		else
			len = endPointer - buffer;
		if(len == 0)
		{
			requestInfo->status = 400;
			return -1;
		}
		//Now we store the parsed resource into our requestInfo
		requestInfo->resource = calloc(len + 1, sizeof(char));
		strncpy(requestInfo->resource,buffer,len);
		//If the buffer is not empty, we know that it was a full HTML request
		if(strstr(buffer, "HTTP/"))
			requestInfo->type = 1;
		else
		{
			//otherwise, we finish parsing
			requestInfo->type = 0;
		}
		firstHeader = 0;
		return 0;
	}
	//if we get here, then we are parsing some other line of the full request
	endPointer = strchr(buffer, ':');
	if(endPointer == NULL)
	{
		requestInfo->status = 400;
		return -1;
	}
	//Use a temporary to hold upper case values to make comparisons easier 
	char *temp = calloc((endPointer - buffer) + 1, sizeof(char));
	strncpy(temp, buffer, (endPointer - buffer));
	StrUpper(temp);
	//Increment buffer, if the buffer ends, just return the 400 status
	buffer = endPointer + 1;
	while(*buffer && isspace(*buffer))
		++buffer;
	if(buffer == '\0')
		return 0;
	if(strcmp(temp, "USER-AGENT") == 0)
	{
		requestInfo->useragent = malloc(strlen(buffer) + 1);
		strcpy(requestInfo->useragent,buffer);
	}
	else if(strcmp(temp, "REFERER") == 0)
	{
		requestInfo->referer = malloc(strlen(buffer)+1);
		strcpy(requestInfo->referer, buffer);
	}
	free(temp);
	return 0;
}

