
// Naive http client
//   by BOT Man & ZhangHan, 2018

#ifndef HTTP_CLIENT
#define HTTP_CLIENT

typedef enum {
  Request_Fd_Limit,       // socket errno == EMFILE || ENFILE
  Request_Socket_Err,     // unknown socket() errors
  Request_Out_Of_Mem,     // out of memory
  Request_Event_New_Err,  // event_new() failed
  Request_Bad_Hostname,   // invalid host or failed in getaddrinfo()
  Request_Conn_Err,       // unknown connect() errors
  Request_Conn_Timeout,   // connect() timeout
  Request_Bad_Sock_Opt,   // invalid sockopt
  Request_Send_Err,       // unknown send() errors
  Request_Send_Timeout,   // send() timeout
  Request_Recv_Err,       // unknown recv() errors
  Request_Recv_Timeout,   // recv() timeout
  Request_Succ,           // HTTP response 200
  Request_Response_Err,   // HTTP response not 200
} RequestStatus;

// async once callback
typedef void (*request_callback_fn)(const char* url,
                                    RequestStatus status,
                                    const char* html,
                                    void* context);

void Request(const char* url, request_callback_fn callback, void* context);

void DispatchLibEvent();
void FreeLibEvent();

#endif  // HTTP_CLIENT
