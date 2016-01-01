#define MAX_REQ_LINE (1024)

struct ReqInfo 
{
    int method;//method -> 0 = GET, 1 = HEAD, -1 = UNSUPPORTED
    int type;//type -> 0 = SIMPLE, 1 = FULL
    char *referer;
    char *useragent;
    char *resource;
    int status;
};

int  parseHTTPHeader(char *buffer, struct ReqInfo *requestInfo);
int  getRequest(int conn, struct ReqInfo *requestInfo);
void initReqInfo(struct ReqInfo *requestInfo);
void freeReqInfo(struct ReqInfo *requestInfo);

/*HTML Header looks like this
METHOD resource

*/
