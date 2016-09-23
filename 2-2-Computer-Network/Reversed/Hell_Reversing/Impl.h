#pragma once

#include <time.h>

#if WIN32
#include <Windows.h>
#else
#include <stdint.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <stdarg.h>
#define Sleep usleep
#endif

#ifdef WIN32
typedef int socklen_t;
typedef int ssize_t;
#else
typedef int SOCKET;
typedef unsigned char BYTE;
typedef unsigned long DWORD;
#define FALSE 0
#define SOCKET_ERROR (-1)
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef uint8_t UINT8;
#define strcmpi strcasecmp
typedef unsigned short u_short;
#endif 


#include <stdio.h>

#define SOCKET_BUF_SIZE 0x20000
#define MAX_TIMER_COUNT 0x80
#define STUFFING_SIZE 0x20

#define STUFFING1 0xF5125A5A
#define STUFFING2 0xA5A5E41B

typedef struct _Socket_Recv
{
	unsigned int size;
	unsigned int flag;
	unsigned char data[2048];
	struct _Socket_Recv *next_ptr;

	// INT32[515]
	// [0] = size
	// [1] = High-4/Low-4 reading flag
	// [2-513] = data (perhaps less)
	// [514] = next_ptr
} Socket_Recv;

extern double c_noise;

extern unsigned int c_B_idle_count;

extern unsigned int c_running_time_limit;

extern unsigned int c_sleep_time;

extern unsigned int is_phl_ready;

extern UINT32 stuffing_1[STUFFING_SIZE];

extern Socket_Recv *p_socket_recved_prev;

extern Socket_Recv *p_socket_recved;

extern unsigned int c_started_time;

extern unsigned int c_recv_total_bit;

extern unsigned int c_socket_buf_up;
extern unsigned int c_socket_buf_low;

extern unsigned char socket_send_buf[SOCKET_BUF_SIZE];

extern unsigned int dbg_mask;

extern time_t c_handshake_time;

extern unsigned int is_flood;

extern unsigned int is_idle_busy_mode;

extern unsigned int c_current_time;

extern SOCKET socket_mine;

extern unsigned char station_type;

extern FILE *p_Log_File;

extern UINT32 stuffing_2[STUFFING_SIZE];

extern unsigned int c_err_count;

extern unsigned int c_phl_capacity;

extern unsigned int is_enable_network;

extern unsigned int is_net_ready;

//=================================================================

extern const char *str_FatalAbort;
extern const char *str_Newline;
