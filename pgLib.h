int strUpper(char* buffer);
ssize_t Readline(int sockd, void *vptr, size_t maxlen);
ssize_t Writeline(int sockd, const void *vptr, size_t n);
int Trim(char * buffer);
void CleanURL(char *buffer);
int outputHTTPHeaders(int conn, struct ReqInfo * reqinfo);
int Return_Error_Msg(int conn, struct ReqInfo * reqinfo);
