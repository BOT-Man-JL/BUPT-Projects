#pragma once

#include <time.h>
#include <Windows.h>
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
