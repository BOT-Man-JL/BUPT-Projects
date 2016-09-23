
#include <time.h>
#include <stdio.h>
#include "Impl.h"

double c_noise = 1.0e-5;

unsigned int c_B_idle_count = 100;

unsigned int c_running_time_limit = 0x7FFFFF00;

unsigned int c_sleep_time = 15;

unsigned int is_phl_ready = 1;

UINT32 stuffing_1[STUFFING_SIZE];

// Array Boundary
//unsigned int c_last_time_long_sleeping;

Socket_Recv *p_socket_recved_prev;

Socket_Recv *p_socket_recved;

unsigned int c_started_time;

// Old Array Boundary
unsigned int c_recv_total_bit;

unsigned int c_socket_buf_up;
unsigned int c_socket_buf_low;

unsigned char socket_send_buf[SOCKET_BUF_SIZE];

unsigned int dbg_mask;

time_t c_handshake_time;

unsigned int is_flood;

unsigned int is_idle_busy_mode;

unsigned int c_current_time;

SOCKET socket_mine;

unsigned char station_type;

FILE *p_Log_File;

UINT32 stuffing_2[STUFFING_SIZE];

// Array Boundary
unsigned int c_err_count;

unsigned int c_phl_capacity;

unsigned int is_enable_network;

unsigned int is_net_ready;

//=================================================================

const char *str_FatalAbort = "FATAL: %s\nAbort.\n";
const char *str_Newline = "\n";
