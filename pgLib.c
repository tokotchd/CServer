#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "html.h"


/*  Read a line from a socket  */
ssize_t Readline(int sockd, void *vptr, size_t maxlen) 
{
    ssize_t n, rc;
    char    c, *buffer;

    buffer = vptr;

    for ( n = 1; n < maxlen; n++ ) 
	{
	
		if ( (rc = read(sockd, &c, 1)) == 1 ) 
		{
			*buffer++ = c;
			if ( c == '\n' )
				break;
		}
		else if ( rc == 0 ) 
		{
			if ( n == 1 )
				return 0;
			else
				break;
		}
		else 
		{
			if ( errno == EINTR )
				continue;
			printf("Error in Readline()");
			exit(-1);
		}
    }

    *buffer = 0;
    return n;
}


/*  Write a line to a socket  */

ssize_t Writeline(int sockd, const void *vptr, size_t n) 
{
    size_t      nleft;
    ssize_t     nwritten;
    const char *buffer;

    buffer = vptr;
    nleft  = n;

    while ( nleft > 0 ) 
	{
		if ( (nwritten = write(sockd, buffer, nleft)) <= 0 ) 
		{
			printf("Error in Writeline()");
			return -1;
		}
	nleft  -= nwritten;
	buffer += nwritten;
    }
    return n;
}
//Outputs HTTP response headers
int outputHTTPHeaders(int conn, struct ReqInfo * reqinfo) 
{
    char buffer[100];

    sprintf(buffer, "HTTP/1.0 %d OK\r\n", reqinfo->status);
    Writeline(conn, buffer, strlen(buffer));
    Writeline(conn, "Server: DTWebServ v1.0\r\n", 24);
    Writeline(conn, "Content-Type: text/html\r\n", 25);
    Writeline(conn, "\r\n", 2);
    return 0;
}

//Cleans up url-encoded strings
void CleanURL(char * buffer) {
    char asciinum[3] = {0};
    int i = 0, c;
    
    while ( buffer[i] ) {
	if ( buffer[i] == '+' )
	    buffer[i] = ' ';
	else if ( buffer[i] == '%' ) {
	    asciinum[0] = buffer[i+1];
	    asciinum[1] = buffer[i+2];
	    buffer[i] = strtol(asciinum, NULL, 16);
	    c = i+1;
	    do {
		buffer[c] = buffer[c+2];
	    } while ( buffer[2+(c++)] );
	}
	++i;
    }

}
//Converts string to upper-case
int StrUpper(char* buffer)
{
	while(*buffer)
	{
		*buffer = toupper(*buffer);
		++buffer;
	}
	return 0;
}



/*  Removes trailing whitespace from a string  */

int Trim(char * buffer) {
    int n = strlen(buffer) - 1;

    while ( !isalnum(buffer[n]) && n >= 0 )
	buffer[n--] = '\0';

    return 0;
}
int Return_Error_Msg(int conn, struct ReqInfo *reqinfo) 
{
    char buffer[100];
    sprintf(buffer, "<HTML>\n<HEAD>\n<TITLE>Server Error %d</TITLE>\n"
	            "</HEAD>\n\n", reqinfo->status);
    Writeline(conn, buffer, strlen(buffer));
    sprintf(buffer, "<BODY>\n<H1>Server Error %d</H1>\n", reqinfo->status);
    Writeline(conn, buffer, strlen(buffer));
    sprintf(buffer, "<P>The request could not be completed.</P>\n"
	            "</BODY>\n</HTML>\n");
    Writeline(conn, buffer, strlen(buffer));
    return 0;
}
